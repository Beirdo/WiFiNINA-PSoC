/*
  WiFiClient.cpp - Library for Arduino Wifi shield ported to C.
  Copyright (c) 2019 Gavin Hurlbut.  All rights reserved.
  Copyright (c) 2018 Arduino SA. All rights reserved.
  Copyright (c) 2011-2014 Arduino LLC.  All right reserved.

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

#include "wl_definitions.h"
#include "wl_types.h"
#include "server_drv.h"
#include "wifi_drv.h"
#include "WifiSocketBuffer.h"

#include "WiFi.h"
#include "WiFiClient.h"

#include "FreeRTOS.h"
#include "task.h"

static int WiFiClient_connectCommon(uint8 _sock);

int WiFiClient_connectHostname(uint8 *host, uint16 port) {
    uint32 remote_addr;
    if (WiFi_hostByName(host, &remote_addr)) {
        return WiFiClient_connect(remote_addr, port, TCP_MODE);
    }
    return 0;
}

int WiFiClient_connect(uint32 ip, uint16 port) {
    uint8 _sock = ServerDrv_getSocket();
    if (_sock == NO_SOCKET_AVAIL) {
        return _sock;
    }

    ServerDrv_startClient(uint32(ip), port, _sock, TCP_MODE);
    return WiFiClient_connectCommon(_sock);
}

static int WiFiClient_connectCommon(uint8 _sock) {
    TickType_t start = xTaskGetTickCount();

    // wait 10 second for the connection to connect
    while (!WiFiClient_connected(_sock) && (xTaskGetTickCount() - start < pdMS_TO_TICKS(10000))) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    if (!WiFiClient_connected(_sock)) {
        return NO_SOCKET_AVAIL;
    }
    return _sock;
}

int WiFiClient_connectSSL(uint32 ip, uint16 port) {
    _sock = ServerDrv_getSocket();
    if (_sock == NO_SOCKET_AVAIL) {
        return _sock;
    }

    ServerDrv_startClient(uint32(ip), port, _sock, TLS_MODE);
    return WiFiClient_connectCommon(_sock);
}

int WiFiClient_connectSSLHostname(uint8 *host, uint16 port) {
    _sock = ServerDrv_getSocket();
    if (_sock == NO_SOCKET_AVAIL) {
        return _sock;
    }

    ServerDrv_startClientHostname(host, strlen(host), uint32(0), port, _sock, TLS_MODE);
    return WiFiClient_connectCommon(_sock);
}

int WiFiClient_writeChar(uint8 _sock, uint8 ch) {
    return WiFiClient_write(_sock, &ch, 1);
}

int WiFiClient_write(uint8 _sock, uint8 *buf, size_t size) {
    if (_sock == NO_SOCKET_AVAIL || size == 0) {
        setWriteError();
        return 0;
    }

    int written = ServerDrv_sendData(_sock, buf, size);
    if (!written || !ServerDrv_checkDataSent(_sock)) {
        setWriteError();
        return 0;
    }

    return written;
}

int WiFiClient_available(uint8 _sock) {
    if (_sock == NO_SOCKET_AVAIL) {
        return 0;
    }

    return WifiSocketBuffer_available(_sock);
}

int WiFiClient_readChar(uint8 _sock) {
    if (!available(_sock)) {
        return -1;
    }

    uint8 ch;
    WifiSocketBuffer_read(_sock, &ch, sizeof(ch));
    return ch;
}

int WiFiClient_read(uint8 *buf, size_t size) {
    return WifiSocketBuffer_read(_sock, buf, size);
}

int WiFiClient_peek(uint8 _sock) {
    return WiFiSocketBuffer_peek(_sock);
}

void WiFiClient_flush(uint8 _sock) {
    // TODO: a real check to ensure transmission has been completed
}

void WiFiClient_stop(uint8 _sock) {
    if (_sock == NO_SOCKET_AVAIL) {
        return;
    }

    ServerDrv_stopClient(_sock);

    int count = 0;

    // wait maximum 5 secs for the connection to close
    while (WiFiClient_status(_sock) != CLOSED && ++count < 50) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    WifiSocketBuffer_close(_sock);
    _sock = NO_SOCKET_AVAIL;
}

int WiFiClient_connected(uint8 _sock) {
    if (_sock == NO_SOCKET_AVAIL) {
        return 0;
    }

    if (WiFiClient_available(_sock)) {
        return 1;
    }

    uint8 s = WiFiClient_status(_sock);

    uint8 result = !(s == LISTEN || s == CLOSED || s == FIN_WAIT_1 || s == FIN_WAIT_2 || s == TIME_WAIT ||
                     s == SYN_SENT || s == SYN_RCVD || s == CLOSE_WAIT);

    if (!result) {
        WiFiSocketBuffer_close(_sock);
        _sock = NO_SOCKET_AVAIL;
    }

    return result;
}

uint8 WiFiClient_status(uint8 _sock) {
    if (_sock == NO_SOCKET_AVAIL) {
        return CLOSED;
    }
    return ServerDrv_getClientState(_sock);
}

uint32 WiFiClient_remoteIP(uint8 _sock) {
    uint32 _remoteIp = 0;
    uint16 _remotePort = 0;

    WiFiDrv_getRemoteData(_sock, (uint8 *)&_remoteIp, (uint8)&_remotePort);
    return ip;
}

uint16 WiFiClient_remotePort() {
    uint32 _remoteIp = 0;
    uint16 _remotePort = 0;

    WiFiDrv_getRemoteData(_sock, (uint8 *)&_remoteIp, (uint8)&_remotePort);
    return _remotePort;
}
