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
char 	WiFiDrv__networkSsid[][WL_SSID_MAX_LENGTH] = {{"1"},{"2"},{"3"},{"4"},{"5"}};

// Cached values of retrieved data
char 	WiFiDrv__ssid[] = {0};
uint8	WiFiDrv__bssid[] = {0};
uint8 WiFiDrv__mac[] = {0};
uint8 WiFiDrv__localIp[] = {0};
uint8 WiFiDrv__subnetMask[] = {0};
uint8 WiFiDrv__gatewayIp[] = {0};
// Firmware version
char    WiFiDrv_fwVersion[] = {0};


/*
 * Get network Data information
 */
static void getNetworkData(uint8 *ip, uint8 *mask, uint8 *gwip);

static uint8 reqHostByName(const char* aHostname);

static int getHostByName(IPAddress *aResult);

/*
 * Get remote Data information on UDP socket
 */
static void getRemoteData(uint8 sock, uint8 *ip, uint8 *port);


// Private Methods
// TODO: example!
static int WiFiDrv_getNetworkData(uint8 *ip, uint8 *mask, uint8 *gwip)
{
    uint8 _dummy = DUMMY_DATA;
    tParam inParams[] = { {sizeof(_dummy), &dummy} };
    tParam outParams[] = { {4, ip}, {4, mask}, {4, gwip} };
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_IPADDR_CMD, 1, inParams);

    // Wait for reply
    return SpiDrv_receiveResponseCmd(GET_IPADDR_CMD, 24, &paramsRead, outParams, 3);
}

static int WiFiDrv_getRemoteData(uint8 sock, uint8 *ip, uint8 *port)
{
    tParam inParams[] = { {sizeof(sock), &sock} };
    tParam outParams[] = { {4, ip}, {2, port} };
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(GET_REMOTE_DATA_CMD, 1, inParams);

    // Wait for reply
    return SpiDrv_receiveResponseCmd(GET_REMOTE_DATA_CMD, 24, &paramsRead, outParams, 2);
}


// Public Methods


void WiFiDrv_wifiDriverInit()
{
    SpiDrv_begin();
}

void WiFiDrv_wifiDriverDeinit()
{
    SpiDrv_end();
}

int8 WiFiDrv_wifiSetNetwork(const char* ssid, uint8 ssid_len)
{
    uint8 _data = 0;
    tParam inParams[] = { {ssid_len, ssid} };
    tParam outParams[] = { {1, &_data} };
    uint8 paramsRead;

    // Send Command
    SpiDrv_sendCmd(SET_NET_CMD, 1, inParams);

    // Wait for reply
    if (!SpiDrv_receiveResponseCmd(SET_NET_CMD, 16, &paramsRead, outParams, 1)
    {
        return WL_FAILURE;
    }

    return(_data == WIFI_SPI_ACK) ? WL_SUCCESS : WL_FAILURE;
}

// TODO: got here
int8 WiFiDrv_wifiSetPassphrase(const char* ssid, uint8 ssid_len, const char *passphrase, const uint8 len)
{
	WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv_sendCmd(SET_PASSPHRASE_CMD, PARAM_NUMS_2);
    SpiDrv_sendParam((uint8*)ssid, ssid_len, NO_LAST_PARAM);
    SpiDrv_sendParam((uint8*)passphrase, len, LAST_PARAM);

    // pad to multiple of 4
    int commandSize = 6 + ssid_len + len;
    while (commandSize % 4) {
        SpiDrv_readChar();
        commandSize++;
    }

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _data = 0;
    uint8 _dataLen = 0;
    if (!SpiDrv_waitResponseCmd(SET_PASSPHRASE_CMD, PARAM_NUMS_1, &_data, &_dataLen))
    {
        WARN("error waitResponse");
        _data = WL_FAILURE;
    }
    SpiDrv_spiSlaveDeselect();
    return _data;
}


int8 WiFiDrv_wifiSetKey(const char* ssid, uint8 ssid_len, uint8 key_idx, const void *key, const uint8 len)
{
	WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv_sendCmd(SET_KEY_CMD, PARAM_NUMS_3);
    SpiDrv_sendParam((uint8*)ssid, ssid_len, NO_LAST_PARAM);
    SpiDrv_sendParam(&key_idx, KEY_IDX_LEN, NO_LAST_PARAM);
    SpiDrv_sendParam((uint8*)key, len, LAST_PARAM);
    
    // pad to multiple of 4
    int commandSize = 8 + ssid_len + len;
    while (commandSize % 4) {
        SpiDrv_readChar();
        commandSize++;
    }

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _data = 0;
    uint8 _dataLen = 0;
    if (!SpiDrv_waitResponseCmd(SET_KEY_CMD, PARAM_NUMS_1, &_data, &_dataLen))
    {
        WARN("error waitResponse");
        _data = WL_FAILURE;
    }
    SpiDrv_spiSlaveDeselect();
    return _data;
}

void WiFiDrv_config(uint8 validParams, uint32 local_ip, uint32 gateway, uint32 subnet)
{
	WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv_sendCmd(SET_IP_CONFIG_CMD, PARAM_NUMS_4);
    SpiDrv_sendParam((uint8*)&validParams, 1, NO_LAST_PARAM);
    SpiDrv_sendParam((uint8*)&local_ip, 4, NO_LAST_PARAM);
    SpiDrv_sendParam((uint8*)&gateway, 4, NO_LAST_PARAM);
    SpiDrv_sendParam((uint8*)&subnet, 4, LAST_PARAM);

    // pad to multiple of 4
    SpiDrv_readChar();
    SpiDrv_readChar();
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _data = 0;
    uint8 _dataLen = 0;
    if (!SpiDrv_waitResponseCmd(SET_IP_CONFIG_CMD, PARAM_NUMS_1, &_data, &_dataLen))
    {
        WARN("error waitResponse");
        _data = WL_FAILURE;
    }
    SpiDrv_spiSlaveDeselect();
}

void WiFiDrv_setDNS(uint8 validParams, uint32 dns_server1, uint32 dns_server2)
{
	WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv_sendCmd(SET_DNS_CONFIG_CMD, PARAM_NUMS_3);
    SpiDrv_sendParam((uint8*)&validParams, 1, NO_LAST_PARAM);
    SpiDrv_sendParam((uint8*)&dns_server1, 4, NO_LAST_PARAM);
    SpiDrv_sendParam((uint8*)&dns_server2, 4, LAST_PARAM);

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _data = 0;
    uint8 _dataLen = 0;
    if (!SpiDrv_waitResponseCmd(SET_DNS_CONFIG_CMD, PARAM_NUMS_1, &_data, &_dataLen))
    {
        WARN("error waitResponse");
        _data = WL_FAILURE;
    }
    SpiDrv_spiSlaveDeselect();
}

void WiFiDrv_setHostname(const char* hostname)
{
    WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv_sendCmd(SET_HOSTNAME_CMD, PARAM_NUMS_1);
    SpiDrv_sendParam((uint8*)hostname, strlen(hostname), LAST_PARAM);

    // pad to multiple of 4
    int commandSize = 5 + strlen(hostname);
    while (commandSize % 4) {
        SpiDrv_readChar();
        commandSize++;
    }

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _data = 0;
    uint8 _dataLen = 0;
    if (!SpiDrv_waitResponseCmd(SET_HOSTNAME_CMD, PARAM_NUMS_1, &_data, &_dataLen))
    {
        WARN("error waitResponse");
        _data = WL_FAILURE;
    }
    SpiDrv_spiSlaveDeselect();
}
                        
int8 WiFiDrv_disconnect()
{
	WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv_sendCmd(DISCONNECT_CMD, PARAM_NUMS_1);

    uint8 _dummy = DUMMY_DATA;
    SpiDrv_sendParam(&_dummy, 1, LAST_PARAM);

    // pad to multiple of 4
    SpiDrv_readChar();
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _data = 0;
    uint8 _dataLen = 0;
    int8 result = SpiDrv_waitResponseCmd(DISCONNECT_CMD, PARAM_NUMS_1, &_data, &_dataLen);

    SpiDrv_spiSlaveDeselect();

    return result;
}

uint8 WiFiDrv_getConnectionStatus()
{
	WAIT_FOR_SLAVE_SELECT();

    // Send Command
    SpiDrv_sendCmd(GET_CONN_STATUS_CMD, PARAM_NUMS_0);

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _data = -1;
    uint8 _dataLen = 0;
    SpiDrv_waitResponseCmd(GET_CONN_STATUS_CMD, PARAM_NUMS_1, &_data, &_dataLen);

    SpiDrv_spiSlaveDeselect();

    return _data;
}

uint8* WiFiDrv_getMacAddress()
{
	WAIT_FOR_SLAVE_SELECT();

    // Send Command
    SpiDrv_sendCmd(GET_MACADDR_CMD, PARAM_NUMS_1);

    uint8 _dummy = DUMMY_DATA;
    SpiDrv_sendParam(&_dummy, 1, LAST_PARAM);
    
    // pad to multiple of 4
    SpiDrv_readChar();
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _dataLen = 0;
    SpiDrv_waitResponseCmd(GET_MACADDR_CMD, PARAM_NUMS_1, _mac, &_dataLen);

    SpiDrv_spiSlaveDeselect();

    return _mac;
}

void WiFiDrv_getIpAddress(IPAddress& ip)
{
	getNetworkData(_localIp, _subnetMask, _gatewayIp);
	ip = _localIp;
}

 void WiFiDrv_getSubnetMask(IPAddress& mask)
 {
	getNetworkData(_localIp, _subnetMask, _gatewayIp);
	mask = _subnetMask;
 }

 void WiFiDrv_getGatewayIP(IPAddress& ip)
 {
	getNetworkData(_localIp, _subnetMask, _gatewayIp);
	ip = _gatewayIp;
 }

const char* WiFiDrv_getCurrentSSID()
{
	WAIT_FOR_SLAVE_SELECT();

    // Send Command
    SpiDrv_sendCmd(GET_CURR_SSID_CMD, PARAM_NUMS_1);

    uint8 _dummy = DUMMY_DATA;
    SpiDrv_sendParam(&_dummy, 1, LAST_PARAM);

    // pad to multiple of 4
    SpiDrv_readChar();
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    memset(_ssid, 0x00, sizeof(_ssid));

    // Wait for reply
    uint8 _dataLen = 0;
    SpiDrv_waitResponseCmd(GET_CURR_SSID_CMD, PARAM_NUMS_1, (uint8*)_ssid, &_dataLen);

    SpiDrv_spiSlaveDeselect();

    return _ssid;
}

uint8* WiFiDrv_getCurrentBSSID()
{
	WAIT_FOR_SLAVE_SELECT();

    // Send Command
    SpiDrv_sendCmd(GET_CURR_BSSID_CMD, PARAM_NUMS_1);

    uint8 _dummy = DUMMY_DATA;
    SpiDrv_sendParam(&_dummy, 1, LAST_PARAM);

    // pad to multiple of 4
    SpiDrv_readChar();
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _dataLen = 0;
    SpiDrv_waitResponseCmd(GET_CURR_BSSID_CMD, PARAM_NUMS_1, _bssid, &_dataLen);

    SpiDrv_spiSlaveDeselect();

    return _bssid;
}

int32 WiFiDrv_getCurrentRSSI()
{
	WAIT_FOR_SLAVE_SELECT();

    // Send Command
    SpiDrv_sendCmd(GET_CURR_RSSI_CMD, PARAM_NUMS_1);

    uint8 _dummy = DUMMY_DATA;
    SpiDrv_sendParam(&_dummy, 1, LAST_PARAM);

    // pad to multiple of 4
    SpiDrv_readChar();
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _dataLen = 0;
    int32 rssi = 0;
    SpiDrv_waitResponseCmd(GET_CURR_RSSI_CMD, PARAM_NUMS_1, (uint8*)&rssi, &_dataLen);

    SpiDrv_spiSlaveDeselect();

    return rssi;
}

uint8 WiFiDrv_getCurrentEncryptionType()
{
	WAIT_FOR_SLAVE_SELECT();

    // Send Command
    SpiDrv_sendCmd(GET_CURR_ENCT_CMD, PARAM_NUMS_1);

    uint8 _dummy = DUMMY_DATA;
    SpiDrv_sendParam(&_dummy, 1, LAST_PARAM);

    // pad to multiple of 4
    SpiDrv_readChar();
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 dataLen = 0;
    uint8 encType = 0;
    SpiDrv_waitResponseCmd(GET_CURR_ENCT_CMD, PARAM_NUMS_1, (uint8*)&encType, &dataLen);

    SpiDrv_spiSlaveDeselect();

    return encType;
}

int8 WiFiDrv_startScanNetworks()
{
	WAIT_FOR_SLAVE_SELECT();

    // Send Command
    SpiDrv_sendCmd(START_SCAN_NETWORKS, PARAM_NUMS_0);

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _data = 0;
    uint8 _dataLen = 0;

    if (!SpiDrv_waitResponseCmd(START_SCAN_NETWORKS, PARAM_NUMS_1, &_data, &_dataLen))
     {
         WARN("error waitResponse");
         _data = WL_FAILURE;
     }

    SpiDrv_spiSlaveDeselect();

    return ((int8)_data == WL_FAILURE)? _data : (int8)WL_SUCCESS;
}


uint8 WiFiDrv_getScanNetworks()
{
	WAIT_FOR_SLAVE_SELECT();

    // Send Command
    SpiDrv_sendCmd(SCAN_NETWORKS, PARAM_NUMS_0);

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 ssidListNum = 0;
    SpiDrv_waitResponse(SCAN_NETWORKS, &ssidListNum, (uint8**)_networkSsid, WL_NETWORKS_LIST_MAXNUM);

    SpiDrv_spiSlaveDeselect();

    return ssidListNum;
}

const char* WiFiDrv_getSSIDNetoworks(uint8 networkItem)
{
	if (networkItem >= WL_NETWORKS_LIST_MAXNUM)
		return (char*)NULL;

	return _networkSsid[networkItem];
}

uint8 WiFiDrv_getEncTypeNetowrks(uint8 networkItem)
{
	if (networkItem >= WL_NETWORKS_LIST_MAXNUM)
		return ENC_TYPE_UNKNOWN;

	WAIT_FOR_SLAVE_SELECT();

    // Send Command
    SpiDrv_sendCmd(GET_IDX_ENCT_CMD, PARAM_NUMS_1);

    SpiDrv_sendParam(&networkItem, 1, LAST_PARAM);

    // pad to multiple of 4
    SpiDrv_readChar();
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 dataLen = 0;
    uint8 encType = 0;
    SpiDrv_waitResponseCmd(GET_IDX_ENCT_CMD, PARAM_NUMS_1, (uint8*)&encType, &dataLen);

    SpiDrv_spiSlaveDeselect();

    return encType;
}

uint8* WiFiDrv_getBSSIDNetowrks(uint8 networkItem, uint8* bssid)
{
    if (networkItem >= WL_NETWORKS_LIST_MAXNUM)
        return NULL;

    WAIT_FOR_SLAVE_SELECT();

    // Send Command
    SpiDrv_sendCmd(GET_IDX_BSSID, PARAM_NUMS_1);

    SpiDrv_sendParam(&networkItem, 1, LAST_PARAM);

    // pad to multiple of 4
    SpiDrv_readChar();
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 dataLen = 0;
    SpiDrv_waitResponseCmd(GET_IDX_BSSID, PARAM_NUMS_1, (uint8*)bssid, &dataLen);

    SpiDrv_spiSlaveDeselect();

    return bssid;  
}

uint8 WiFiDrv_getChannelNetowrks(uint8 networkItem)
{
    if (networkItem >= WL_NETWORKS_LIST_MAXNUM)
        return 0;

    WAIT_FOR_SLAVE_SELECT();

    // Send Command
    SpiDrv_sendCmd(GET_IDX_CHANNEL_CMD, PARAM_NUMS_1);

    SpiDrv_sendParam(&networkItem, 1, LAST_PARAM);

    // pad to multiple of 4
    SpiDrv_readChar();
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 dataLen = 0;
    uint8 channel = 0;
    SpiDrv_waitResponseCmd(GET_IDX_CHANNEL_CMD, PARAM_NUMS_1, (uint8*)&channel, &dataLen);

    SpiDrv_spiSlaveDeselect();

    return channel;  
}

int32 WiFiDrv_getRSSINetoworks(uint8 networkItem)
{
	if (networkItem >= WL_NETWORKS_LIST_MAXNUM)
		return 0;
	int32	networkRssi = 0;

	WAIT_FOR_SLAVE_SELECT();

    // Send Command
    SpiDrv_sendCmd(GET_IDX_RSSI_CMD, PARAM_NUMS_1);

    SpiDrv_sendParam(&networkItem, 1, LAST_PARAM);

    // pad to multiple of 4
    SpiDrv_readChar();
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 dataLen = 0;
    SpiDrv_waitResponseCmd(GET_IDX_RSSI_CMD, PARAM_NUMS_1, (uint8*)&networkRssi, &dataLen);

    SpiDrv_spiSlaveDeselect();

	return networkRssi;
}

uint8 WiFiDrv_reqHostByName(const char* aHostname)
{
	WAIT_FOR_SLAVE_SELECT();

    // Send Command
    SpiDrv_sendCmd(REQ_HOST_BY_NAME_CMD, PARAM_NUMS_1);
    SpiDrv_sendParam((uint8*)aHostname, strlen(aHostname), LAST_PARAM);

    // pad to multiple of 4
    int commandSize = 5 + strlen(aHostname);
    while (commandSize % 4) {
        SpiDrv_readChar();
        commandSize++;
    }

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _data = 0;
    uint8 _dataLen = 0;
    uint8 result = SpiDrv_waitResponseCmd(REQ_HOST_BY_NAME_CMD, PARAM_NUMS_1, &_data, &_dataLen);

    SpiDrv_spiSlaveDeselect();

    if (result) {
        result = (_data == 1);
    }

    return result;
}

int WiFiDrv_getHostByName(IPAddress& aResult)
{
	uint8  _ipAddr[WL_IPV4_LENGTH];
	IPAddress dummy(0xFF,0xFF,0xFF,0xFF);
	int result = 0;

	WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv_sendCmd(GET_HOST_BY_NAME_CMD, PARAM_NUMS_0);

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _dataLen = 0;
    if (!SpiDrv_waitResponseCmd(GET_HOST_BY_NAME_CMD, PARAM_NUMS_1, _ipAddr, &_dataLen))
    {
        WARN("error waitResponse");
    }else{
    	aResult = _ipAddr;
    	result = (aResult != dummy);
    }
    SpiDrv_spiSlaveDeselect();
    return result;
}

int WiFiDrv_getHostByName(const char* aHostname, IPAddress& aResult)
{
	if (reqHostByName(aHostname))
	{
		return getHostByName(aResult);
	}else{
		return 0;
	}
}

const char*  WiFiDrv_getFwVersion()
{
	WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv_sendCmd(GET_FW_VERSION_CMD, PARAM_NUMS_0);

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _dataLen = 0;
    if (!SpiDrv_waitResponseCmd(GET_FW_VERSION_CMD, PARAM_NUMS_1, (uint8*)fwVersion, &_dataLen))
    {
        WARN("error waitResponse");
    }
    SpiDrv_spiSlaveDeselect();
    return fwVersion;
}

uint32 WiFiDrv_getTime()
{
    WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv_sendCmd(GET_TIME_CMD, PARAM_NUMS_0);

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _dataLen = 0;
    uint32 _data = 0;
    if (!SpiDrv_waitResponseCmd(GET_TIME_CMD, PARAM_NUMS_1, (uint8*)&_data, &_dataLen))
    {
        WARN("error waitResponse");
    }
    SpiDrv_spiSlaveDeselect();
    return _data;
}

void WiFiDrv_setPowerMode(uint8 mode)
{
    WAIT_FOR_SLAVE_SELECT();

    // Send Command
    SpiDrv_sendCmd(SET_POWER_MODE_CMD, PARAM_NUMS_1);

    SpiDrv_sendParam(&mode, 1, LAST_PARAM);

    // pad to multiple of 4
    SpiDrv_readChar();
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 dataLen = 0;
    uint8 data = 0;
    SpiDrv_waitResponseCmd(SET_POWER_MODE_CMD, PARAM_NUMS_1, &data, &dataLen);

    SpiDrv_spiSlaveDeselect();
}

int8 WiFiDrv_wifiSetApNetwork(const char* ssid, uint8 ssid_len, uint8 channel)
{
    WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv_sendCmd(SET_AP_NET_CMD, PARAM_NUMS_2);
    SpiDrv_sendParam((uint8*)ssid, ssid_len);
    SpiDrv_sendParam(&channel, 1, LAST_PARAM);

    // pad to multiple of 4
    int commandSize = 3 + ssid_len;
    while (commandSize % 4) {
        SpiDrv_readChar();
        commandSize++;
    }

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _data = 0;
    uint8 _dataLen = 0;
    if (!SpiDrv_waitResponseCmd(SET_AP_NET_CMD, PARAM_NUMS_1, &_data, &_dataLen))
    {
        WARN("error waitResponse");
        _data = WL_FAILURE;
    }
    SpiDrv_spiSlaveDeselect();

    return(_data == WIFI_SPI_ACK) ? WL_SUCCESS : WL_FAILURE;
}

int8 WiFiDrv_wifiSetApPassphrase(const char* ssid, uint8 ssid_len, const char *passphrase, const uint8 len, uint8 channel)
{
    WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv_sendCmd(SET_AP_PASSPHRASE_CMD, PARAM_NUMS_3);
    SpiDrv_sendParam((uint8*)ssid, ssid_len, NO_LAST_PARAM);
    SpiDrv_sendParam((uint8*)passphrase, len, NO_LAST_PARAM);
    SpiDrv_sendParam(&channel, 1, LAST_PARAM);

    // pad to multiple of 4
    int commandSize = 4 + ssid_len + len;
    while (commandSize % 4) {
        SpiDrv_readChar();
        commandSize++;
    }

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _data = 0;
    uint8 _dataLen = 0;
    if (!SpiDrv_waitResponseCmd(SET_AP_PASSPHRASE_CMD, PARAM_NUMS_1, &_data, &_dataLen))
    {
        WARN("error waitResponse");
        _data = WL_FAILURE;
    }
    SpiDrv_spiSlaveDeselect();
    return _data;
}

int16 WiFiDrv_ping(uint32 ipAddress, uint8 ttl)
{
    WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv_sendCmd(PING_CMD, PARAM_NUMS_2);
    SpiDrv_sendParam((uint8*)&ipAddress, sizeof(ipAddress), NO_LAST_PARAM);
    SpiDrv_sendParam((uint8*)&ttl, sizeof(ttl), LAST_PARAM);

    // pad to multiple of 4
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint16 _data;
    uint8 _dataLen = 0;
    if (!SpiDrv_waitResponseCmd(PING_CMD, PARAM_NUMS_1, (uint8*)&_data, &_dataLen))
    {
        WARN("error waitResponse");
        _data = WL_PING_ERROR;
    }
    SpiDrv_spiSlaveDeselect();
    return _data;  
}

void WiFiDrv_debug(uint8 on)
{
    WAIT_FOR_SLAVE_SELECT();

    // Send Command
    SpiDrv_sendCmd(SET_DEBUG_CMD, PARAM_NUMS_1);

    SpiDrv_sendParam(&on, 1, LAST_PARAM);

    // pad to multiple of 4
    SpiDrv_readChar();
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 dataLen = 0;
    uint8 data = 0;
    SpiDrv_waitResponseCmd(SET_DEBUG_CMD, PARAM_NUMS_1, &data, &dataLen);

    SpiDrv_spiSlaveDeselect(); 
}

float WiFiDrv_getTemperature()
{
    WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv_sendCmd(GET_TEMPERATURE_CMD, PARAM_NUMS_0);

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _dataLen = 0;
    float _data = 0;
    if (!SpiDrv_waitResponseCmd(GET_TEMPERATURE_CMD, PARAM_NUMS_1, (uint8*)&_data, &_dataLen))
    {
        WARN("error waitResponse");
    }
    SpiDrv_spiSlaveDeselect();
    return _data;
}

void WiFiDrv_pinMode(uint8 pin, uint8 mode)
{
    WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv_sendCmd(SET_PIN_MODE, PARAM_NUMS_2);
    SpiDrv_sendParam((uint8*)&pin, 1, NO_LAST_PARAM);
    SpiDrv_sendParam((uint8*)&mode, 1, LAST_PARAM);

    // pad to multiple of 4
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _data = 0;
    uint8 _dataLen = 0;
    if (!SpiDrv_waitResponseCmd(SET_PIN_MODE, PARAM_NUMS_1, &_data, &_dataLen))
    {
        WARN("error waitResponse");
        _data = WL_FAILURE;
    }
    SpiDrv_spiSlaveDeselect();
}

void WiFiDrv_digitalWrite(uint8 pin, uint8 value)
{
    WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv_sendCmd(SET_DIGITAL_WRITE, PARAM_NUMS_2);
    SpiDrv_sendParam((uint8*)&pin, 1, NO_LAST_PARAM);
    SpiDrv_sendParam((uint8*)&value, 1, LAST_PARAM);

    // pad to multiple of 4
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _data = 0;
    uint8 _dataLen = 0;
    if (!SpiDrv_waitResponseCmd(SET_DIGITAL_WRITE, PARAM_NUMS_1, &_data, &_dataLen))
    {
        WARN("error waitResponse");
        _data = WL_FAILURE;
    }
    SpiDrv_spiSlaveDeselect();
}

void WiFiDrv_analogWrite(uint8 pin, uint8 value)
{
    WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv_sendCmd(SET_ANALOG_WRITE, PARAM_NUMS_2);
    SpiDrv_sendParam((uint8*)&pin, 1, NO_LAST_PARAM);
    SpiDrv_sendParam((uint8*)&value, 1, LAST_PARAM);

    // pad to multiple of 4
    SpiDrv_readChar();

    SpiDrv_spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv_waitForSlaveReady();
    SpiDrv_spiSlaveSelect();

    // Wait for reply
    uint8 _data = 0;
    uint8 _dataLen = 0;
    if (!SpiDrv_waitResponseCmd(SET_ANALOG_WRITE, PARAM_NUMS_1, &_data, &_dataLen))
    {
        WARN("error waitResponse");
        _data = WL_FAILURE;
    }
    SpiDrv_spiSlaveDeselect();
}

WiFiDrv wiFiDrv;
