/*
  server_drv.h - Library for Arduino Wifi shield ported to C.
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

#ifndef Server_Drv_h
#define Server_Drv_h

#include "project.h"
#include "wifi_spi.h"

typedef enum eProtMode {
    TCP_MODE, UDP_MODE, TLS_MODE, UDP_MULTICAST_MODE
} tProtMode;

// Start server TCP on port specified
int ServerDrv_startServer(uint16 port, uint8 sock, uint8 protMode);

int ServerDrv_startServerIpAddress(uint32 ipAddress, uint16 port, uint8 sock, uint8 protMode);

int ServerDrv_startClient(uint32 ipAddress, uint16 port, uint8 sock, uint8 protMode);

int ServerDrv_startClientHostname(uint8 *host, uint8 host_len, uint32 ipAddress, uint16 port, uint8 sock,
                                  uint8 protMode);

int ServerDrv_stopClient(uint8 sock);

int ServerDrv_getServerState(uint8 sock);

int ServerDrv_getClientState(uint8 sock);

int ServerDrv_getData(uint8 sock, uint8 *data, uint8 peek);

int ServerDrv_getDataBuf(uint8 sock, uint8 *data, uint16 *len);

int ServerDrv_insertDataBuf(uint8 sock, uint8 *_data, uint16 _dataLen);

int ServerDrv_sendData(uint8 sock, uint8 *data, uint16 len);

int ServerDrv_sendUdpData(uint8 sock);

int ServerDrv_availData(uint8 sock);

int ServerDrv_checkDataSent(uint8 sock);

int ServerDrv_getSocket();

#endif
