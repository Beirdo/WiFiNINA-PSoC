/*
  server_drv.cpp - Library for Arduino Wifi shield ported to C.
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

#include "server_drv.h"

#include "project.h"
#include "spi_drv.h"
#include "wl_types.h"

#include "FreeRTOS.h"
#include "task.h"

// Start server TCP on port specified
int ServerDrv_startServer(uint16 port, uint8 sock, uint8 protMode) {
    uint8 _data = 0;
    tParam inParams[] = {{2, &port},
                         {1, &sock},
                         {1, &protMode}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(START_SERVER_TCP_CMD, 3, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(START_SERVER_TCP_CMD, 16, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }
    return _data;
}

int ServerDrv_startServerIpAddress(uint32 ipAddress, uint16 port, uint8 sock, uint8 protMode) {
    uint8 _data = 0;
    tParam inParams[] = {{4, &ipAddress},
                         {2, &port},
                         {1, &sock},
                         {1, &protMode}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(START_SERVER_TCP_CMD, 4, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(START_SERVER_TCP_CMD, 16, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }
    return _data;
}

// Start server TCP on port specified
int ServerDrv_startClient(uint32 ipAddress, uint16 port, uint8 sock, uint8 protMode) {
    uint8 _data = 0;
    tParam inParams[] = {{4, &ipAddress},
                         {2, &port},
                         {1, &sock},
                         {1, &protMode}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(START_CLIENT_TCP_CMD, 4, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(START_CLIENT_TCP_CMD, 16, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }
    return _data;
}

int ServerDrv_startClientHostname(uint8 *host, uint8 host_len, uint32 ipAddress, uint16 port, uint8 sock, uint8 protMode) {
    uint8 _data = 0;
    tParam inParams[] = {{host_len, host},
                         {4,        &ipAddress},
                         {2,        &port},
                         {1,        &sock},
                         {1,        &protMode}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(START_CLIENT_TCP_CMD, 5, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(START_CLIENT_TCP_CMD, 16, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }
    return _data;
}

// Start server TCP on port specified
int ServerDrv_stopClient(uint8 sock) {
    uint8 _data = 0;
    tParam inParams[] = {{1, &sock}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(STOP_CLIENT_TCP_CMD, 1, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(STOP_CLIENT_TCP_CMD, 16, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }
    return _data;
}


int ServerDrv_getServerState(uint8 sock) {
    uint8 _data = 0;
    tParam inParams[] = {{1, &sock}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_STATE_TCP_CMD, 1, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(GET_STATE_TCP_CMD, 16, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }
    return _data;
}

int ServerDrv_getClientState(uint8 sock) {
    uint8 _data = 0;
    tParam inParams[] = {{1, &sock}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_CLIENT_STATE_TCP_CMD, 1, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(GET_CLIENT_STATE_TCP_CMD, 16, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }
    return _data;
}

int ServerDrv_availData(uint8 sock) {
    uint16 _data = 0;
    tParam inParams[] = {{1, &sock}};
    tParam outParams[] = {{2, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(AVAIL_DATA_TCP_CMD, 1, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(AVAIL_DATA_TCP_CMD, 20, &paramsRead, outParams, 1);
    return _data;
}


int ServerDrv_getData(uint8 sock, uint8 *data, uint8 peek) {
    tParam inParams[] = {{1, &sock},
                         {1, &peek}};
    tParam outParams[] = {{WIFI_SOCKET_BUFFER_SIZE, &data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_DATA_TCP_CMD, 1, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(GET_DATA_TCP_CMD, WIFI_SOCKET_BUFFER_SIZE, &paramsRead, outParams, 1);
    return outParams[0].paramLen;
}

int ServerDrv_getDataBuf(uint8 sock, uint8 *_data, uint16 *_dataLen) {
    tDataParam inParams[] = {{1, &sock},
                             {2, _dataLen}};
    tDataParam outParams[] = {{*_dataLen, _data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendBuffer(GET_DATABUF_TCP_CMD, 2, inParams);

    // Wait for reply
    SpiDrv_receiveResponseBuffer(GET_DATABUF_TCP_CMD, WIFI_SOCKET_BUFFER_SIZE, &paramsRead, outParams, 1);
    return outParams[0].dataLen;
}

int ServerDrv_insertDataBuf(uint8 sock, const uint8 *data, uint16 _len) {
    int16 response = 0;
    tDataParam inParams[] = {{1,    &sock},
                             {_len, data}};
    tDataParam outParams[] = {{2, &response}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendBuffer(INSERT_DATABUF_CMD, 2, inParams);

    // Wait for reply
    SpiDrv_receiveResponseBuffer(INSERT_DATABUF_CMD, 20, &paramsRead, outParams, 1);
    if (outParams[0].dataLen == 0) {
        return 0;
    }
    return (response == 1);
}

int ServerDrv_sendUdpData(uint8 sock) {
    uint8 response = 0;
    tParam inParams[] = {{1, &sock}};
    tParam outParams[] = {{1, &response}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(SEND_DATA_UDP_CMD, 1, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(SEND_DATA_UDP_CMD, 20, &paramsRead, outParams, 1);
    if (outParams[0].dataLen == 0) {
        return 0;
    }
    return (response == 1);
}

int ServerDrv_sendTcpData(uint8 sock, const uint8 *data, uint16 len) {
    uint8 response = 0;
    tDataParam inParams[] = {{1,   &sock},
                             {len, data}};
    tDataParam outParams[] = {{1, &response}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendBuffer(SEND_DATA_TCP_CMD, 2, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(SEND_DATA_TCP_CMD, 20, &paramsRead, outParams, 1);
    return response;
}

int ServerDrv_checkDataSent(uint8 sock) {
    const uint16 TIMEOUT_DATA_SENT = 25;
    uint16 timeout = 0;
    uint8 response = 0;
    tParam inParams[] = {{1, &sock}};
    tParam outParams[] = {{1, &response}};
    uint8 paramsRead;

    do {
        // Send Command
        SpiDrv_sendCmd(DATA_SENT_TCP_CMD, 1, inParams);

        // Wait for reply
        SpiDrv_receiveResponseCmd(DATA_SENT_TCP_CMD, 20, &paramsRead, outParams, 1);
        if (response) {
            timeout = 0;
        } else {
            ++timeout;
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    } while ((response == 0) && (timeout < TIMEOUT_DATA_SENT));
    return (timeout != TIMEOUT_DATA_SENT)l
}

int ServerDrv_getSocket() {
    uint8 response = 0;
    tParam inParams[] = {};
    tParam outParams[] = {{1, &response}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_SOCKET_CMD, 0, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(GET_SOCKET_CMD, 20, &paramsRead, outParams, 1);
    return response;
}
