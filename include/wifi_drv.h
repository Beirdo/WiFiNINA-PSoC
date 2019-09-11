/*
  wifi_drv.h - Library for Arduino Wifi shield ported to C.
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

#ifndef WiFi_Drv_h
#define WiFi_Drv_h

#include "project.h"
#include "wifi_spi.h"
#include "IPAddress.h"
#include "WiFiUdp.h"
#include "WiFiClient.h"

// Key index length
#define KEY_IDX_LEN     1
// 5 secs of delay to have the connection established
#define WL_DELAY_START_CONNECTION 5000
// firmware version string length
#define WL_FW_VER_LENGTH 6

/*
 * Driver initialization
 */
void WiFiDrv_wifiDriverInit();

void WiFiDrv_wifiDriverDeinit();

/*
 * Set the desired network which the connection manager should try to
 * connect to.
 *
 * The ssid of the desired network should be specified.
 *
 * param ssid: The ssid of the desired network.
 * param ssid_len: Length of ssid string.
 * return: WL_SUCCESS or WL_FAILURE
 */
int8 WiFiDrv_wifiSetNetwork(const char* ssid, uint8 ssid_len);

/* Start Wifi connection with passphrase
 * the most secure supported mode will be automatically selected
 *
 * param ssid: Pointer to the SSID string.
 * param ssid_len: Lenght of ssid string.
 * param passphrase: Passphrase. Valid characters in a passphrase
 *        must be between ASCII 32-126 (decimal).
 * param len: Lenght of passphrase string.
 * return: WL_SUCCESS or WL_FAILURE
 */
int8 WiFiDrv_wifiSetPassphrase(const char* ssid, uint8 ssid_len, const char *passphrase, const uint8 len);

/* Start Wifi connection with WEP encryption.
 * Configure a key into the device. The key type (WEP-40, WEP-104)
 * is determined by the size of the key (5 bytes for WEP-40, 13 bytes for WEP-104).
 *
 * param ssid: Pointer to the SSID string.
 * param ssid_len: Lenght of ssid string.
 * param key_idx: The key index to set. Valid values are 0-3.
 * param key: Key input buffer.
 * param len: Lenght of key string.
 * return: WL_SUCCESS or WL_FAILURE
 */
int8 WiFiDrv_wifiSetKey(const char* ssid, uint8 ssid_len, uint8 key_idx, const void *key, const uint8 len);

int8 WiFiDrv_wifiSetApNetwork(const char* ssid, uint8 ssid_len);
int8 WiFiDrv_wifiSetApPassphrase(const char* ssid, uint8 ssid_len, const char *passphrase, const uint8 len);

/* Set ip configuration disabling dhcp client
    *
    * param validParams: set the number of parameters that we want to change
    * 					 i.e. validParams = 1 means that we'll change only ip address
    * 					 	  validParams = 3 means that we'll change ip address, gateway and netmask
    * param local_ip: 	ip configuration
    * param gateway: 	gateway configuration
    * param subnet: 	subnet mask configuration
    */
void WiFiDrv_config(uint8 validParams, uint32 local_ip, uint32 gateway, uint32 subnet);

/* Set DNS ip configuration
       *
       * param validParams: set the number of parameters that we want to change
       * 					 i.e. validParams = 1 means that we'll change only dns_server1
       * 					 	  validParams = 2 means that we'll change dns_server1 and dns_server2
       * param dns_server1: DNS server1 configuration
       * param dns_server2: DNS server2 configuration
       */
void WiFiDrv_setDNS(uint8 validParams, uint32 dns_server1, uint32 dns_server2);

void WiFiDrv_setHostname(const char* hostname);

/*
 * Disconnect from the network
 *
 * return: WL_SUCCESS or WL_FAILURE
 */
int8 WiFiDrv_disconnect();

/*
 * Disconnect from the network
 *
 * return: one value of wl_status_t enum
 */
uint8 WiFiDrv_getConnectionStatus();

/*
 * Get the interface MAC address.
 *
 * return: pointer to uint8 array with length WL_MAC_ADDR_LENGTH
 */
uint8* WiFiDrv_getMacAddress();

/*
 * Get the interface IP address.
 *
 * return: copy the ip address value in IPAddress object
 */
void WiFiDrv_getIpAddress(IPAddress *ip);

/*
 * Get the interface subnet mask address.
 *
 * return: copy the subnet mask address value in IPAddress object
 */
void WiFiDrv_getSubnetMask(IPAddress *mask);

/*
 * Get the gateway ip address.
 *
 * return: copy the gateway ip address value in IPAddress object
 */
void WiFiDrv_getGatewayIP(IPAddress *ip);

/*
 * Return the current SSID associated with the network
 *
 * return: ssid string
 */
const char* WiFiDrv_getCurrentSSID();

/*
 * Return the current BSSID associated with the network.
 * It is the MAC address of the Access Point
 *
 * return: pointer to uint8 array with length WL_MAC_ADDR_LENGTH
 */
uint8* WiFiDrv_getCurrentBSSID();

/*
 * Return the current RSSI /Received Signal Strength in dBm)
 * associated with the network
 *
 * return: signed value
 */
int32 WiFiDrv_getCurrentRSSI();

/*
 * Return the Encryption Type associated with the network
 *
 * return: one value of wl_enc_type enum
 */
uint8 WiFiDrv_getCurrentEncryptionType();

/*
 * Start scan WiFi networks available
 *
 * return: Number of discovered networks
 */
int8 WiFiDrv_startScanNetworks();

/*
 * Get the networks available
 *
 * return: Number of discovered networks
 */
uint8 WiFiDrv_getScanNetworks();

/*
 * Return the SSID discovered during the network scan.
 *
 * param networkItem: specify from which network item want to get the information
 *
 * return: ssid string of the specified item on the networks scanned list
 */
const char* WiFiDrv_getSSIDNetoworks(uint8 networkItem);

/*
 * Return the RSSI of the networks discovered during the scanNetworks
 *
 * param networkItem: specify from which network item want to get the information
 *
 * return: signed value of RSSI of the specified item on the networks scanned list
 */
int32 WiFiDrv_getRSSINetoworks(uint8 networkItem);

/*
 * Return the encryption type of the networks discovered during the scanNetworks
 *
 * param networkItem: specify from which network item want to get the information
 *
 * return: encryption type (enum wl_enc_type) of the specified item on the networks scanned list
 */
uint8 WiFiDrv_getEncTypeNetowrks(uint8 networkItem);

uint8* WiFiDrv_getBSSIDNetowrks(uint8 networkItem, uint8* bssid);

uint8 WiFiDrv_getChannelNetowrks(uint8 networkItem);

/*
 * Resolve the given hostname to an IP address.
 * param aHostname: Name to be resolved
 * param aResult: IPAddress structure to store the returned IP address
 * result: 1 if aIPAddrString was successfully converted to an IP address,
 *          else error code
 */
int WiFiDrv_getHostByName(const char* aHostname, IPAddress *aResult);

/*
 * Get the firmware version
 * result: version as string with this format a.b.c
 */
const char* WiFiDrv_getFwVersion();

uint32 WiFiDrv_getTime();

void WiFiDrv_setPowerMode(uint8 mode);

int8 WiFiDrv_wifiSetApNetwork(const char* ssid, uint8 ssid_len, uint8 channel);
int8 WiFiDrv_wifiSetApPassphrase(const char* ssid, uint8 ssid_len, const char *passphrase, const uint8 len, uint8 channel);

int16 WiFiDrv_ping(uint32 ipAddress, uint8 ttl);

void WiFiDrv_debug(uint8 on);
float WiFiDrv_getTemperature();
void WiFiDrv_pinMode(uint8 pin, uint8 mode);
void WiFiDrv_digitalWrite(uint8 pin, uint8 value);
void WiFiDrv_analogWrite(uint8 pin, uint8 value);

#endif
