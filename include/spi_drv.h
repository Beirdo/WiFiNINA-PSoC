/*
  spi_drv.h - Library for Arduino Wifi shield ported to C.
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

#ifndef SPI_Drv_h
#define SPI_Drv_h

#include "project.h"
#include "wifi_spi.h"
#include "FreeRTOS.h"

#define DUMMY_DATA  0xFF

// Due to RxBuffer size limitations in the SPIM module.  I can probably work out making this 1500 later.
#define WIFI_SOCKET_BUFFER_SIZE 255

void SpiDrv_begin(void);

void SpiDrv_end(void);

void SpiDrv_waitForSlaveSelect(void);

void SpiDrv_spiSlaveSelect(void);

void SpiDrv_spiSlaveDeselect(void);

uint8 SpiDrv_readChar();

void SpiDrv_waitForSlaveReady();

void SpiDrv_waitForSlaveReadyTimeout(TickType_t timeout);

void SpiDrv_sendBuffer(uint8 cmd, uint8 numParam, tDataParam *params);

int SpiDrv_receiveResponseBuffer(uint8 cmd, uint16 maxSize, uint8 *numParamRead, tDataParam *params, uint8 maxNumParams);

void SpiDrv_sendCmd(uint8 cmd, uint8 numParam, tParam *params);

int SpiDrv_receiveResponseCmd(uint8 cmd, uint16 maxSize, uint8 *numParamRead, tParam *params, uint8 maxNumParams);

#endif
