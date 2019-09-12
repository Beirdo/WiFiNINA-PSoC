/*
  wifi_drv.cpp - Library for Arduino Wifi shield ported to C.
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
#include "wifi_spi.h"
#include "wl_types.h"

// Array of data to cache the information related to the networks discovered
uint8 WiFiDrv__networkSsid[WL_NETWORKS_LIST_MAXNUM][WL_SSID_MAX_LENGTH];

// Cached values of retrieved data
uint8 WiFiDrv__ssid[WL_SSID_MAX_LENGTH] = {0};
uint8 WiFiDrv__bssid[WL_MAC_ADDR_LENGTH] = {0};
uint8 WiFiDrv__mac[WL_MAC_ADDR_LENGTH] = {0};
uint32 WiFiDrv__localIp = 0;
uint32 WiFiDrv__subnetMask = 0;
uint32 WiFiDrv__gatewayIp = 0;

// Firmware version
uint8 WiFiDrv_fwVersion[WL_FW_VER_LENGTH] = {0};


/*
 * Get network Data information
 */
static int WiFiDrv_getNetworkData(uint8 *ip, uint8 *mask, uint8 *gwip);

static int WiFiDrv_reqHostByName(uint8 *aHostname);

static int WiFiDrv_getHostByNameResults(uint32 *aResult);


// Private Methods
static int WiFiDrv_getNetworkData(uint8 *ip, uint8 *mask, uint8 *gwip) {
    uint8 _dummy = DUMMY_DATA;
    tParam inParams[] = {{sizeof(_dummy), &_dummy}};
    tParam outParams[] = {{4, ip},
                          {4, mask},
                          {4, gwip}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_IPADDR_CMD, 1, inParams);

    // Wait for reply
    return SpiDrv_receiveResponseCmd(GET_IPADDR_CMD, 24, &paramsRead, outParams, 3);
}

int WiFiDrv_getRemoteData(uint8 sock, uint8 *ip, uint8 *port) {
    tParam inParams[] = {{sizeof(sock), &sock}};
    tParam outParams[] = {{4, ip},
                          {2, port}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_REMOTE_DATA_CMD, 1, inParams);

    // Wait for reply
    return SpiDrv_receiveResponseCmd(GET_REMOTE_DATA_CMD, 24, &paramsRead, outParams, 2);
}


// Public Methods


void WiFiDrv_wifiDriverInit(void) {
    SpiDrv_begin();
}

void WiFiDrv_wifiDriverDeinit(void) {
    SpiDrv_end();
}

int WiFiDrv_wifiSetNetwork(uint8 *ssid, uint8 ssid_len) {
    uint8 _data = 0;
    tParam inParams[] = {{ssid_len, ssid}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(SET_NET_CMD, 1, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(SET_NET_CMD, 16, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }

    return (_data == WIFI_SPI_ACK) ? WL_SUCCESS : WL_FAILURE;
}

int WiFiDrv_wifiSetPassphrase(uint8 *ssid, uint8 ssid_len, uint8 *passphrase, uint8 len) {
    uint8 _data = 0;
    tParam inParams[] = {{ssid_len, ssid},
                         {len,      passphrase}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(SET_PASSPHRASE_CMD, 2, inParams);
    if (!SpiDrv_receiveResponseCmd(SET_PASSPHRASE_CMD, 16, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }
    return _data;
}


int WiFiDrv_wifiSetKey(uint8 *ssid, uint8 ssid_len, uint8 key_idx, uint8 *key, uint8 len) {
    uint8 _data = 0;
    tParam inParams[] = {{ssid_len,    ssid},
                         {KEY_IDX_LEN, &key_idx},
                         {len,         key}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(SET_KEY_CMD, 3, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(SET_KEY_CMD, 16, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }
    return _data;
}

int WiFiDrv_config(uint8 validParams, uint32 local_ip, uint32 gateway, uint32 subnet) {
    uint8 _data = 0;
    tParam inParams[] = {{1, &validParams},
                         {4, &local_ip},
                         {4, &gateway},
                         {4, &subnet}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(SET_IP_CONFIG_CMD, 4, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(SET_IP_CONFIG_CMD, 16, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }
    return _data;
}

int WiFiDrv_setDNS(uint8 validParams, uint32 dns_server1, uint32 dns_server2) {
    uint8 _data = 0;
    tParam inParams[] = {{1, &validParams},
                         {4, &dns_server1},
                         {4, &dns_server2}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(SET_DNS_CONFIG_CMD, 3, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(SET_DNS_CONFIG_CMD, 16, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }
    return _data;
}

int WiFiDrv_setHostname(uint8 *hostname) {
    uint8 _data = 0;
    tParam inParams[] = {{ustrlen(hostname), hostname}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(SET_HOSTNAME_CMD, 1, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(SET_HOSTNAME_CMD, 16, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }
    return _data;
}

int WiFiDrv_disconnect(void) {
    uint8 _data = 0;
    uint8 _dummy = DUMMY_DATA;
    tParam inParams[] = {{1, &_dummy}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(DISCONNECT_CMD, 1, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(DISCONNECT_CMD, 16, &paramsRead, outParams, 1);
    return _data;
}

int WiFiDrv_getConnectionStatus(void) {
    int8 _data = -1;
    tParam inParams[] = {};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_CONN_STATUS_CMD, 0, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(GET_CONN_STATUS_CMD, 16, &paramsRead, outParams, 1);
    return _data;
}

uint8 *WiFiDrv_getMacAddress(void) {
    uint8 _dummy = DUMMY_DATA;
    tParam inParams[] = {{1, &_dummy}};
    tParam outParams[] = {{WL_MAC_ADDR_LENGTH, WiFiDrv__mac}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_MACADDR_CMD, 1, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(GET_MACADDR_CMD, 32, &paramsRead, outParams, 1);
    return WiFiDrv__mac;
}

int WiFiDrv_getIpAddress(uint32 *ip) {
    int retVal = WiFiDrv_getNetworkData(WiFiDrv__localIp, WiFiDrv__subnetMask, WiFiDrv__gatewayIp);
    *ip = WiFiDrv__localIp;
    return retVal;
}

int WiFiDrv_getSubnetMask(uint32 *mask) {
    int retVal = WiFiDrv_getNetworkData(WiFiDrv__localIp, WiFiDrv__subnetMask, WiFiDrv__gatewayIp);
    *mask = WiFiDrv__subnetMask;
    return retVal;
}

int WiFiDrv_getGatewayIP(uint32 *ip) {
    int retVal = WiFiDrv_getNetworkData(WiFiDrv__localIp, WiFiDrv__subnetMask, WiFiDrv__gatewayIp);
    *ip = WiFiDrv__gatewayIp;
    return retVal;
}

uint8 *WiFiDrv_getCurrentSSID(void) {
    uint8 _dummy = DUMMY_DATA;
    tParam inParams[] = {{1, &_dummy}};
    tParam outParams[] = {{WL_SSID_MAX_LENGTH, WiFiDrv__ssid}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_CURR_SSID_CMD, 1, inParams);

    memset(WiFiDrv__ssid, 0x00, WL_SSID_MAX_LENGTH);

    // Wait for reply
    SpiDrv_receiveResponseCmd(GET_CURR_SSID_CMD, 48, &paramsRead, outParams, 1);
    return WiFiDrv__ssid;
}

uint8 *WiFiDrv_getCurrentBSSID(void) {
    uint8 _dummy = DUMMY_DATA;
    tParam inParams[] = {{1, &_dummy}};
    tParam outParams[] = {{WL_MAC_ADDR_LENGTH, WiFiDrv__bssid}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_CURR_BSSID_CMD, 1, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(GET_CURR_BSSID_CMD, 32, &paramsRead, outParams, 1);
    return WiFiDrv__bssid;
}

int32 WiFiDrv_getCurrentRSSI(void) {
    uint8 _dummy = DUMMY_DATA;
    int32 rssi;
    tParam inParams[] = {{1, &_dummy}};
    tParam outParams[] = {{4, &rssi}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_CURR_RSSI_CMD, 1, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(GET_CURR_RSSI_CMD, 20, &paramsRead, outParams, 1);
    return rssi;
}

int WiFiDrv_getCurrentEncryptionType(void) {
    uint8 encType = 0;
    uint8 _dummy = DUMMY_DATA;
    tParam inParams[] = {{1, &_dummy}};
    tParam outParams[] = {{1, &encType}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_CURR_ENCT_CMD, 1, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(GET_CURR_ENCT_CMD, 20, &paramsRead, outParams, 1);
    return encType;
}

int WiFiDrv_startScanNetworks(void) {
    int8 _data = 0;
    tParam inParams[] = {};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(START_SCAN_NETWORKS, 0, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(START_SCAN_NETWORKS, 16, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }

    if (_data == WL_FAILURE) {
        return WL_FAILURE;
    }
    return WL_SUCCESS;
}

int WiFiDrv_getScanNetworks(void) {
    int i;
    tParam inParams[] = {};
    tParam outParams[WL_NETWORKS_LIST_MAXNUM];
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(SCAN_NETWORKS, 0, inParams);

    memset(WiFiDrv__networkSsid, 0, sizeof(WiFiDrv__networkSsid));

    for (i = 0; i < WL_NETWORKS_LIST_MAXNUM; i++) {
        outParams[i].paramLen = WL_SSID_MAX_LENGTH;
        outParams[i].param = WiFiDrv__networkSsid[i];
    }

    // Wait for reply
    SpiDrv_receiveResponseCmd(SCAN_NETWORKS, WIFI_SOCKET_BUFFER_SIZE, &paramsRead, outParams, WL_NETWORKS_LIST_MAXNUM);
    return paramsRead;
}

uint8 *WiFiDrv_getSSIDNetworks(uint8 networkItem) {
    if (networkItem >= WL_NETWORKS_LIST_MAXNUM)
        return (uint8 *) NULL;

    return &WiFiDrv__networkSsid[networkItem];
}

int WiFiDrv_getEncTypeNetworks(uint8 networkItem) {
    uint8 encType = 0;
    tParam inParams[] = {{1, &networkItem}};
    tParam outParams[] = {{1, &encType}};
    uint8 paramsRead;

    if (networkItem >= WL_NETWORKS_LIST_MAXNUM)
        return ENC_TYPE_UNKNOWN;

    // Send Command
    SpiDrv_sendCmd(GET_IDX_ENCT_CMD, 1, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(GET_IDX_ENCT_CMD, 20, &paramsRead, outParams, 1);
    return encType;
}

uint8 *WiFiDrv_getBSSIDNetworks(uint8 networkItem, uint8 *bssid) {
    tParam inParams[] = {{1, &networkItem}};
    tParam outParams[] = {{WL_MAC_ADDR_LENGTH, bssid}};
    uint8 paramsRead;

    if (networkItem >= WL_NETWORKS_LIST_MAXNUM)
        return NULL;

    // Send Command
    SpiDrv_sendCmd(GET_IDX_BSSID, 1, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(GET_IDX_BSSID, 32, &paramsRead, outParams, 1);
    return bssid;
}

int WiFiDrv_getChannelNetworks(uint8 networkItem) {
    uint8 channel = 0;
    tParam inParams[] = {{1, &networkItem}};
    tParam outParams[] = {{1, &channel}};
    uint8 paramsRead;

    if (networkItem >= WL_NETWORKS_LIST_MAXNUM)
        return 0;

    // Send Command
    SpiDrv_sendCmd(GET_IDX_CHANNEL_CMD, 1, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(GET_IDX_CHANNEL_CMD, 20, &paramsRead, outParams, 1);
    return channel;
}

int32 WiFiDrv_getRSSINetworks(uint8 networkItem) {
    int32 rssi = 0;
    tParam inParams[] = {{1, &networkItem}};
    tParam outParams[] = {{4, &rssi}};
    uint8 paramsRead;

    if (networkItem >= WL_NETWORKS_LIST_MAXNUM)
        return 0;

    // Send Command
    SpiDrv_sendCmd(GET_IDX_RSSI_CMD, 1, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(GET_IDX_CHANNEL_CMD, 20, &paramsRead, outParams, 1);
    return rssi;
}

static int WiFiDrv_reqHostByName(uint8 *aHostname) {
    uint8 _data = 0;
    uint8 result;
    tParam inParams[] = {{ustrlen(aHostname), aHostname}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(REQ_HOST_BY_NAME_CMD, 1, inParams);

    // Wait for reply
    result = SpiDrv_receiveResponseCmd(REQ_HOST_BY_NAME_CMD, 20, &paramsRead, outParams, 1);

    if (result) {
        result = (_data == 1);
    }

    return result;
}

static int WiFiDrv_getHostByNameResults(uint32 *aResult) {
    uint32 _ipAddr;
    uint32 dummy = 0xFFFFFFFF;
    int result = 0;
    tParam inParams[] = {};
    tParam outParams[] = {{4, &_ipAddr}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_HOST_BY_NAME_CMD, 0, inParams);

    // Wait for reply
    result = SpiDrv_receiveResponseCmd(GET_HOST_BY_NAME_CMD, 32, &paramsRead, outParams, 1);
    if (result) {
        *aResult = _ipAddr;
        result = (_ipAddr != dummy);
    }
    return result;
}

int WiFiDrv_getHostByName(uint8 *aHostname, uint32 *aResult) {
    if (WiFiDrv_reqHostByName(aHostname)) {
        return WiFiDrv_getHostByNameResults(aResult);
    } else {
        return 0;
    }
}

uint8 *WiFiDrv_getFwVersion(void) {
    tParam inParams[] = {};
    tParam outParams[] = {{WL_FW_VER_LENGTH, WiFiDrv_fwVersion}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_FW_VERSION_CMD, 0, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(GET_FW_VERSION_CMD, 48, &paramsRead, outParams, 1);
    return WiFiDrv_fwVersion;
}

uint32 WiFiDrv_getTime(void) {
    uint32 _data = 0;
    tParam inParams[] = {};
    tParam outParams[] = {{4, &_data}};
    uint8 paramsRead;

    // Send command
    SpiDrv_sendCmd(GET_TIME_CMD, 0, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(GET_TIME_CMD, 48, &paramsRead, outParams, 1);
    return _data;
}

int WiFiDrv_setPowerMode(uint8 mode) {
    uint8 _data = 0;
    tParam inParams[] = {{1, &mode}};
    tParam outParams[] = {{4, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(SET_POWER_MODE_CMD, 1, inParams);

    // Wait for reply
    return SpiDrv_receiveResponseCmd(SET_POWER_MODE_CMD, 20, &paramsRead, outParams, 1);
}

int WiFiDrv_wifiSetApNetwork(uint8 *ssid, uint8 ssid_len, uint8 channel) {
    uint8 _data = 0;
    tParam inParams[] = {{ssid_len, ssid}, {1, &channel}};
    tParam outParams[] = {{4, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(SET_AP_NET_CMD, 2, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(SET_AP_NET_CMD, 20, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }

    return (_data == WIFI_SPI_ACK) ? WL_SUCCESS : WL_FAILURE;
}

int WiFiDrv_wifiSetApPassphrase(uint8 *ssid, uint8 ssid_len, uint8 *passphrase, uint8 len, uint8 channel) {
    uint8 _data = 0;
    tParam inParams[] = {{ssid_len, ssid}, {len, passphrase}, {1, &channel}};
    tParam outParams[] = {{4, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(SET_AP_PASSPHRASE_CMD, 3, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(SET_AP_PASSPHRASE_CMD, 20, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }
    return _data;
}

int16 WiFiDrv_ping(uint32 ipAddress, uint8 ttl) {
    int16 _data;
    tParam inParams[] = {{4, &ipAddress}, {1, &ttl}};
    tParam outParams[] = {{2, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(PING_CMD, 2, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(PING_CMD, 24, &paramsRead, outParams, 1)) {
        return WL_PING_ERROR;
    }

    return _data;
}

int WiFiDrv_debug(uint8 on) {
    uint8 _data = 0;
    tParam inParams[] = {{1, &on}};
    tParam outParams[] = {{2, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(SET_DEBUG_CMD, 1, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(SET_DEBUG_CMD, 24, &paramsRead, outParams, 1);
    return data;
}

int16 WiFiDrv_getTemperature(void) {
    float _data = 0.0;
    tParam inParams[] = {};
    tParam outParams[] = {{sizeof(float), &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_TEMPERATURE_CMD, 0, inParams);

    // Wait for reply
    SpiDrv_receiveResponseCmd(GET_TEMPERATURE_CMD, 24, &paramsRead, outParams, 1);
    return (int16)(_data * 8.0);
}

int WiFiDrv_pinMode(uint8 pin, uint8 mode) {
    uint8 _data = 0;
    tParam inParams[] = {{1, &pin}, {1, &mode}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(SET_PIN_MODE, 2, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(SET_PIN_MODE, 24, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }
    return _data;
}

int WiFiDrv_digitalWrite(uint8 pin, uint8 value) {
    uint8 _data = 0;
    tParam inParams[] = {{1, &pin}, {1, &value}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(SET_DIGITAL_WRITE, 2, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(SET_DIGITAL_WRITE, 24, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }
    return _data;
}

int WiFiDrv_analogWrite(uint8 pin, uint8 value) {
    uint8 _data = 0;
    tParam inParams[] = {{1, &pin}, {1, &value}};
    tParam outParams[] = {{1, &_data}};
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(SET_ANALOG_WRITE, 2, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(SET_ANALOG_WRITE, 24, &paramsRead, outParams, 1)) {
        return WL_FAILURE;
    }
    return _data;
}
