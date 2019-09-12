/*
  spi_drv.c - Library for Arduino Wifi shield ported to C.
  Copyright (c) 2019 Gavin Hurlbut.  All rights reserved.
  Copyright (c) 2018 Arduino SA. All rights reserved.
  Copyright (c) 2011-2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "project.h"
#include "spi_drv.h"
#include "wifi_drv.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t slaveReadyDetected;
SemaphoreHandle_t spiTxCompleted;
BaseType_t spiTxPreempted;

#define SPI_MAX_TX_BUFFER 255   // hope there are no responses or commands bigger.
#define SPI_MAX_RX_BUFFER 255   // hope there are no responses or commands bigger.

static bool SpiDrv_initialized = false;
static uint8 txBuffer[SPI_MAX_TX_BUFFER];


static int SpiDrv_waitSpiChar(uint8 waitChar);

static int SpiDrv_readAndCheckChar(uint8 checkChar, uint8 *readChar);

// PSoC interrupt on falling edge of ESPBUSY - triggers a FreeRTOS semaphore
void ESP_BUSY_IRQ_Interrupt_InterruptCallback(void) {
    static BaseType_t preempted = pdFALSE;
    xSemaphoreGiveFromISR(slaveReadyDetected, &preempted);
    portYIELD_FROM_ISR(preempted);
}

// PSoC interrupt fpr SPI Tx.  We only care when transfer is complete
void SPIM_WIFI_TX_ISR_EntryCallback(void) {
    spiTxPreempted = pdFALSE;
    if ((SPIM_WIFI_STATUS & SPIM_WIFI_STATUS_MASK) & SPIM_WIFI_INT_ON_SPI_DONE) {
        xSemaphoreGiveFromISR(spiTxCompleted, &spiTxPreempted);
    }
}

void SPIM_WIFI_TX_ISR_ExitCallback(void) {
    portYIELD_FROM_ISR(spiTxPreempted);
}

void SpiDrv_begin(void) {
    slaveReadyDetected = xSemaphoreCreateBinary();

    ESPRST_WRITE(1);
    vTaskDelay(pdMS_TO_TICKS(10));
    ESPRST_Write(0);
    vTaskDelay(pdMS_TO_TICKS(10));
    ESPRST_WRITE(1);
    vTaskDelay(pdMS_TO_TICKS(750));

    initialized = true;
}

void SpiDrv_end(void) {
    ESPRST_Write(0);
    WIFI_CS_OVERRIDE_write(0);

    initialized = false;
}


void SpiDrv_waitForSlaveSelect(void) {
    if (!SpiDrv_initialized) {
        SpiDrv_begin();
    }
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();
}

void SpiDrv_spiSlaveSelect(void) {
    // Actual SPI slave select is built-in.  Override it so we can see if the slave is ready
    WIFI_CS_OVERRIDE_Write(1);
    SpiDrv_waitForSlaveReadyTimeout(pdMS_TO_TICKS(5));
}

void SpiDrv_spiSlaveDeselect(void) {
    WIFI_CS_OVERRIDE_Write(0);
}

static int SpiDrv_waitSpiChar(uint8 waitChar) {
    int timeout = TIMEOUT_CHAR;
    unsigned char _readChar = 0;
    do {
        _readChar = SpiDrv_readChar(); //get data byte
        if (_readChar == ERR_CMD) {
            return -1;
        }
    } while ((timeout-- > 0) && (_readChar != waitChar));
    return (_readChar == waitChar);
}

static int SpiDrv_readAndCheckChar(uint8 checkChar, uint8 *readChar) {
    *readChar = SpiDrv_readChar();
    return (*readChar == checkChar);
}

uint8 SpiDrv_readChar() {
    if (SPIM_WIFI_GetRxBufferSize() == 0) {
        return 0;
    }
    return SPIM_WIFI_ReadRxData();
}

void SpiDrv_waitForSlaveReady() {
    SpiDrv_waitForSlaveReadyTimeout(pdMAX_DELAY);

    // Shouldn't happen
    while (ESPBUSY_Read()) {};
}

void SpiDrv_waitForSlaveReadyTimeout(TickType_t timeout) {
    if (ESPBUSY_Read()) {
        xSemaphoreTake(slaveReadyDetected, timeout);
    }
}

void SpiDrv_sendBuffer(uint8 cmd, uint8 numParam, tDataParam *params) {
    int i;
    int j;

    txBuffer[0] = START_CMD;
    txBuffer[1] = cmd & ~(REPLY_FLAG);
    // totlen seems to not be used
    txBuffer[2] = numParam;

    for (i = 0, j = 3; i < numParam; i++) {
        tDataParam *param = &params[i];
        uint16 len = param->paramLen;
        txBuffer[j++] = (len >> 8) & 0xFF;
        txBuffer[j++] = len & 0xFF;
        memcpy(param->param, &txBuffer[j], len);
        j += len;
    }

    // Want a total buffer length of integer multiple of 32
    while ((j & 3) != 3) {
        txBuffer[j++] = 0;
    }
    txBuffer[j]++ = END_CMD;

    // Make sure the TX and RX buffer are cleared
    SPIM_WIFI_ClearTxBuffer();
    SPIM_WIFI_ClearRxBuffer();

    SpiDrv_waitForSlaveSelect();
    SPIM_WIFI_PutArray(txBuffer, j);
    xSemaphoreTake(spiTxCompleted, pdMAX_DELAY);
    SpiDrv_spiSlaveDeselect();
}

int SpiDrv_receiveResponseBuffer(uint8 cmd, uint16 maxSize, uint8 *numParamRead, tDataParams *params, uint8 maxNumParams) {
    char _data = 0;

    if (maxSize > SPI_MAX_RX_BUFFER) {
        maxSize = SPI_MAX_RX_BUFFER;
    }

    // Make sure the TX and RX buffer are cleared
    SPIM_WIFI_ClearTxBuffer();
    SPIM_WIFI_ClearRxBuffer();

    // Wait the reply elaboration
    SpiDrv_waitForSlaveSelect();

    // Unfortunately, to receive, we must transmit.  Transmit all zeros.
    memset(txBuffer, 0, maxSize);
    SPIM_WIFI_PutArray(txBuffer, j);
    xSemaphoreTake(spiTxCompleted, pdMAX_DELAY);
    SpiDrv_spiSlaveDeselect();

    // The data will come into the rx buffer as we Tx, so let's pull from it.
    if (!SpiDrv_waitSpiChar(START_CMD)) {
        return 0;
    }

    if (!readAndCheckChar(cmd | REPLY_FLAG, &_data)) {
        return 0;
    }

    uint8 numParam = SpiDrv_readChar();

    if (numParam > maxNumParams) {
        numParam = maxNumParams;
    }

    *numParamRead = numParam;
    if (numParam == 0) {
        return 0;
    }

    for (i = 0; i < numParam; ++i) {
        tDataParams *param = &params[i];
        uint8 *buf = param->data;
        uint16 len = ((SpiDrv_readChar() & 0xFF) << 8);
        len |= (SpiDrv_readChar() & 0xFF);

        if (len >= param->dataLen) {
            len = param->dataLen - 1;
        }

        for (ii = 0; ii < len; ii++) {
            *buf++ = SpiDrv_getChar();
        }
        *buf = 0;
        param->dataLen = len;
    }

    return readAndCheckChar(END_CMD, &_data);
}

/* Cmd Struct Message */
/* _________________________________________________________________________________  */
/*| START CMD | C/R  | CMD  |[TOT LEN]| N.PARAM | PARAM LEN | PARAM  | .. | END CMD | */
/*|___________|______|______|_________|_________|___________|________|____|_________| */
/*|   8 bit   | 1bit | 7bit |  8bit   |  8bit   |   8bit    | nbytes | .. |   8bit  | */
/*|___________|______|______|_________|_________|___________|________|____|_________| */

void SpiDrv_sendCmd(uint8 cmd, uint8 numParam, tParam *params) {
    int i;
    int j;

    txBuffer[0] = START_CMD;
    txBuffer[1] = cmd & ~(REPLY_FLAG);
    // totlen seems to not be used
    txBuffer[2] = numParam;

    for (i = 0, j = 3; i < numParam; i++) {
        tParam *param = &params[i];
        uint8 len = param->paramLen;
        txBuffer[j++] = lenen;
        memcpy(param->param, &txBuffer[j], len);
        j += len;
    }

    // Want a total buffer length of integer multiple of 32
    while ((j & 3) != 3) {
        txBuffer[j++] = 0;
    }
    txBuffer[j]++ = END_CMD;

    // Make sure the TX and RX buffer are cleared
    SPIM_WIFI_ClearTxBuffer();
    SPIM_WIFI_ClearRxBuffer();

    SpiDrv_waitForSlaveSelect();
    SPIM_WIFI_PutArray(txBuffer, j);
    xSemaphoreTake(spiTxCompleted, pdMAX_DELAY);
    SpiDrv_spiSlaveDeselect();
}

int SpiDrv_receiveResponseCmd(uint8 cmd, uint16 maxSize, uint8 *numParamRead, tParams *params, uint8 maxNumParams) {
    char _data = 0;

    if (maxSize > SPI_MAX_RX_BUFFER) {
        maxSize = SPI_MAX_RX_BUFFER;
    }

    // Make sure the TX and RX buffer are cleared
    SPIM_WIFI_ClearTxBuffer();
    SPIM_WIFI_ClearRxBuffer();

    // Wait the reply elaboration
    SpiDrv_waitForSlaveSelect();

    // Unfortunately, to receive, we must transmit.  Transmit all zeros.
    memset(txBuffer, 0, maxSize);
    SPIM_WIFI_PutArray(txBuffer, j);
    xSemaphoreTake(spiTxCompleted, pdMAX_DELAY);
    SpiDrv_spiSlaveDeselect();

    // The data will come into the rx buffer as we Tx, so let's pull from it.
    if (!SpiDrv_waitSpiChar(START_CMD)) {
        return 0;
    }

    if (!readAndCheckChar(cmd | REPLY_FLAG, &_data)) {
        return 0;
    }

    uint8 numParam = SpiDrv_readChar();

    if (numParam > maxNumParams) {
        numParam = maxNumParams;
    }

    *numParamRead = numParam;
    if (numParam == 0) {
        return 0;
    }

    for (i = 0; i < numParam; ++i) {
        tParams *param = &params[i];
        uint8 *buf = param->param;
        uint8 len = SpiDrv_readChar();

        if (len >= param->paramLen) {
            len = param->paramLen - 1;
        }

        for (ii = 0; ii < len; ii++) {
            *buf++ = SpiDrv_getChar();
        }
        *buf = 0;
        param->paramLen = len;
    }

    return readAndCheckChar(END_CMD, &_data);
}
