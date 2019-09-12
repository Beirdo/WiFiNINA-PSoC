/*
  This file is part of the WiFiNINA library ported to C.
  Copyright (c) 2019 Gavin Hurlbut.  All rights reserved.
  Copyright (c) 2018 Arduino SA. All rights reserved.

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef WiFiSocketBuffer_h
#define WiFiSocketBuffer_h

#include "project.h"
#include "wl_definitions.h"

typedef struct _WiFiSocketBuffer {
    uint8* data;
    uint8* head;
    int length;
} WiFiSocketBuffer_t;

void WiFiSocketBuffer_init(void);
void WiFiSocketBuffer_deinit(void);

void WiFiSocketBuffer_close(int socket);

int WiFiSocketBuffer_available(int socket);
int WiFiSocketBuffer_peek(int socket);
int WiFiSocketBuffer_read(int socket, uint8* data, size_t length);

#endif
