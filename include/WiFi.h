/*
  WiFi.h - Library for Arduino Wifi shield ported to C.
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

#ifndef WiFi_h
#define WiFi_h

#define WIFI_FIRMWARE_LATEST_VERSION "1.2.1"

#include "project.h"

#include "wl_definitions.h"
#include "wl_types.h"
#include "WiFiClient.h"

void WiFi_init();

/*
 * Get firmware version
 */
uint8 *WiFi_firmwareVersion();


/* Start Wifi connection for OPEN networks
 *
 * param ssid: Pointer to the SSID string.
 */
int WiFi_begin_open(uint8 *ssid);

/* Start Wifi connection with WEP encryption.
 * Configure a key into the device. The key type (WEP-40, WEP-104)
 * is determined by the size of the key (5 bytes for WEP-40, 13 bytes for WEP-104).
 *
 * param ssid: Pointer to the SSID string.
 * param key_idx: The key index to set. Valid values are 0-3.
 * param key: Key input buffer.
 */
int WiFi_begin_WEP(uint8 *ssid, uint8 key_idx, uint8 *key);

/* Start Wifi connection with passphrase
 * the most secure supported mode will be automatically selected
 *
 * param ssid: Pointer to the SSID string.
 * param passphrase: Passphrase. Valid characters in a passphrase
 *        must be between ASCII 32-126 (decimal).
 */
int WiFi_begin_passphrase(uint8 *ssid, uint8 *passphrase);

uint8 WiFi_beginAP_ssid(uint8 *ssid);

uint8 WiFi_beginAP_ssid_channel(uint8 *ssid, uint8 channel);

uint8 WiFi_beginAP_ssid_passphrase(uint8 *ssid, uint8 *passphrase);

uint8 WiFi_beginAP_ssid_passphrase_channel(uint8 *ssid, uint8 *passphrase, uint8 channel);

/* Change Ip configuration settings disabling the dhcp client
    *
    * param local_ip: 	Static ip configuration
    */
void WiFi_config_static(uint32 local_ip);

/* Change Ip configuration settings disabling the dhcp client
    *
    * param local_ip: 	Static ip configuration
* param dns_server:     IP configuration for DNS server 1
    */
void WiFi_config_static_dns(uint32 local_ip, uint32 dns_server);

/* Change Ip configuration settings disabling the dhcp client
    *
    * param local_ip: 	Static ip configuration
* param dns_server:     IP configuration for DNS server 1
    * param gateway : 	Static gateway configuration
    */
void WiFi_config_static_dns_gateway(uint32 local_ip, uint32 dns_server, uint32 gateway);

/* Change Ip configuration settings disabling the dhcp client
    *
    * param local_ip: 	Static ip configuration
* param dns_server:     IP configuration for DNS server 1
    * param gateway: 	Static gateway configuration
    * param subnet:		Static Subnet mask
    */
void
WiFi_config_static_dns_gateway_subnet(uint32 local_ip, uint32 dns_server, uint32 gateway, uint32 subnet);

/* Change DNS Ip configuration
 *
 * param dns_server1: ip configuration for DNS server 1
 */
void WiFi_setDNS(uint32 dns_server1);

/* Change DNS Ip configuration
 *
 * param dns_server1: ip configuration for DNS server 1
 * param dns_server2: ip configuration for DNS server 2
 *
 */
void WiFi_setDNS_two_servers(uint32 dns_server1, uint32 dns_server2);


/* Set the hostname used for DHCP requests
 *
 * param name: hostname to set
 *
 */
void WiFi_setHostname(uint8 *name);

/*
 * Disconnect from the network
 *
 * return: one value of wl_status_t enum
 */
int WiFi_disconnect(void);

void WiFi_end(void);

/*
 * Get the interface MAC address.
 *
 * return: pointer to uint8 array with length WL_MAC_ADDR_LENGTH
 */
uint8 *WiFi_macAddress(uint8 *mac);

/*
 * Get the interface IP address.
 *
 * return: Ip address value
 */
uint32 WiFi_localIP();

/*
 * Get the interface subnet mask address.
 *
 * return: subnet mask address value
 */
uint32 WiFi_subnetMask();

/*
 * Get the gateway ip address.
 *
 * return: gateway ip address value
 */
uint32 WiFi_gatewayIP();

/*
 * Return the current SSID associated with the network
 *
 * return: ssid string
 */
uint8 *WiFi_SSID();

/*
  * Return the current BSSID associated with the network.
  * It is the MAC address of the Access Point
  *
  * return: pointer to uint8 array with length WL_MAC_ADDR_LENGTH
  */
uint8 *WiFi_BSSID(uint8 *bssid);

/*
  * Return the current RSSI /Received Signal Strength in dBm)
  * associated with the network
  *
  * return: signed value
  */
int32 WiFi_RSSI();

/*
  * Return the Encryption Type associated with the network
  *
  * return: one value of wl_enc_type enum
  */
uint8 WiFi_encryptionType();

/*
 * Start scan WiFi networks available
 *
 * return: Number of discovered networks
 */
int8 WiFi_scanNetworks();

/*
 * Return the SSID discovered during the network scan.
 *
 * param networkItem: specify from which network item want to get the information
 *
 * return: ssid string of the specified item on the networks scanned list
 */
uint8 *WiFi_SSID_index(uint8 networkItem);

/*
 * Return the encryption type of the networks discovered during the scanNetworks
 *
 * param networkItem: specify from which network item want to get the information
 *
 * return: encryption type (enum wl_enc_type) of the specified item on the networks scanned list
 */
uint8 WiFi_encryptionType_index(uint8 networkItem);

uint8 *WiFi_BSSID_index(uint8 networkItem, uint8 *bssid);

uint8 WiFi_channel_index(uint8 networkItem);

/*
 * Return the RSSI of the networks discovered during the scanNetworks
 *
 * param networkItem: specify from which network item want to get the information
 *
 * return: signed value of RSSI of the specified item on the networks scanned list
 */
int32 WiFi_RSSI_index(uint8 networkItem);

/*
 * Return Connection status.
 *
 * return: one of the value defined in wl_status_t
 */
uint8 WiFi_status();

/*
 * Resolve the given hostname to an IP address.
 * param aHostname: Name to be resolved
 * param aResult: pointer to uint32 structure to store the returned IP address
 * result: 1 if aIPAddrString was successfully converted to an IP address,
 *          else error code
 */
int WiFi_hostByName(uint8 *aHostname, uint32 *aResult);

unsigned long WiFi_getTime();

void WiFi_lowPowerMode();

void WiFi_noLowPowerMode();

int WiFi_ping_hostname(uint8 *hostname, uint8 ttl);

int WiFi_ping_ipaddress(uint32 host, uint8 ttl);

void WiFi_setLEDs(uint8 red, uint8 green, uint8 blue);

#endif
