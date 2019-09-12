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

#ifndef wificlient_h
#define wificlient_h

#include "project.h"

uint8 WiFiClient_status();

int WiFiClient_connectHostname(uint32 ip, uint16 port);

int WiFiClient_connect(uint8 *host, uint16 port);

int WiFiClient_connectSSL(uint32 ip, uint16 port);

int WiFiClient_connectSSLHostname(uint8 *host, uint16 port);

int WiFiClient_writeChar(uint8 _sock, uint8 ch);

int WiFiClient_write(uint8 _sock, uint8 *buf, size_t size);

int WiFiClient_available(uint8 _sock);

int WiFiClient_readChar(uint8 _sock);

int WiFiClient_read(uint8 _sock, uint8 *buf, size_t size);

int WiFiClient_peek(uint8 _sock);

void WiFiClient_flush(uint8 _sock);

void WiFiClient_stop(uint8 _sock);

int WiFiClient_connected(uint8 _sock);

uint32 WiFiClient_remoteIP(uint8 _sock);

uint16 WiFiClient_remotePort(uint8 _sock);

#endif
