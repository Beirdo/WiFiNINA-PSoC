/*
  WiFi.c - Library for Arduino Wifi shield ported to C.
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

#include "project.h"
#include "wifi_drv.h"
#include "WiFi.h"

extern SPIClass *WIFININA_SPIWIFI;
extern int8 WIFININA_SLAVESELECT, WIFININA_SLAVEREADY, WIFININA_SLAVERESET, WIFININA_SLAVEGPIO0;

#include "wl_definitions.h"
#include "wl_types.h"


void WiFi_setLEDs(uint8 red, uint8 green, uint8 blue) {
    WiFiDrv_pinMode(25, OUTPUT);
    WiFiDrv_pinMode(26, OUTPUT);
    WiFiDrv_pinMode(27, OUTPUT);
    WiFiDrv_analogWrite(25, red);
    WiFiDrv_analogWrite(26, green);
    WiFiDrv_analogWrite(27, blue);
}

void WiFi_init() {
    WiFiDrv_wifiDriverInit();
}

const char *WiFi_firmwareVersion() {
    return WiFiDrv_getFwVersion();
}

int WiFi_begin_common(void) {
    uint8 status = WL_IDLE_STATUS;
    uint8 attempts = WL_MAX_ATTEMPT_CONNECTION;

    do {
        delay(WL_DELAY_START_CONNECTION);
        status = WiFiDrv_getConnectionStatus();
    } while (((status == WL_IDLE_STATUS) || (status == WL_NO_SSID_AVAIL) || (status == WL_SCAN_COMPLETED)) &&
             (--attempts > 0));
    return status;

}

int WiFi_begin_open(const char *ssid) {
    if (WiFiDrv_wifiSetNetwork(ssid, strlen(ssid)) == WL_FAILURE) {
        return WL_CONNECT_FAILED;
    }
    return WiFi_begin_common();
}

int WiFi_begin_WEP(const char *ssid, uint8 key_idx, const char *key) {
    // set encryption key
    if (WiFiDrv_wifiSetKey(ssid, strlen(ssid), key_idx, key, strlen(key)) == WL_FAILURE) {
        status = WL_CONNECT_FAILED;
    }
    return WiFi_begin_common();
}

int WiFi_begin(const char *ssid, const char *passphrase) {
    // set passphrase
    if (WiFiDrv_wifiSetPassphrase(ssid, strlen(ssid), passphrase, strlen(passphrase)) == WL_FAILURE) {
        status = WL_CONNECT_FAILED;
    }
    return WiFi_begin_common();
}

uint8 WiFi_beginAP_common(void) {
    uint8 status = WL_IDLE_STATUS;
    uint8 attempts = WL_MAX_ATTEMPT_CONNECTION;

    do {
        delay(WL_DELAY_START_CONNECTION);
        status = WiFiDrv_getConnectionStatus();
    } while (((status == WL_IDLE_STATUS) || (status == WL_SCAN_COMPLETED)) && (--attempts > 0));
    return status;
}

uint8 WiFi_beginAP_ssid(const char *ssid) {
    return WiFi_beginAP_ssid_channel(ssid, 1);
}

uint8 WiFi_beginAP_ssid_channel(const char *ssid, uint8 channel) {
    if (WiFiDrv_wifiSetApNetwork(ssid, strlen(ssid), channel) == WL_FAILURE) {
        return WL_AP_FAILED;
    }
    return WiFi_beginAP_common();
}

uint8 WiFi_beginAP_ssid_channel(const char *ssid, const char *passphrase) {
    return WiFi_beginAP_ssid_passphrase_channel(ssid, passphrase, 1);
}

uint8 WiFi_beginAP_ssid_passphrase_channel(const char *ssid, const char *passphrase, uint8 channel) {
    // set passphrase
    if (WiFiDrv_wifiSetApPassphrase(ssid, strlen(ssid), passphrase, strlen(passphrase), channel) == WL_FAILURE) {
        return WL_AP_FAILED;
    }
    return WiFi_beginAP_common();
}

void WiFi_config_static(IPAddress local_ip) {
    WiFiDrv_config(1, (uint32) local_ip, 0, 0);
}

void WiFi_config_static_dns(IPAddress local_ip, IPAddress dns_server) {
    WiFiDrv_config(1, (uint32) local_ip, 0, 0);
    WiFiDrv_setDNS(1, (uint32) dns_server, 0);
}

void WiFi_config_static_dns_gateway(IPAddress local_ip, IPAddress dns_server, IPAddress gateway) {
    WiFiDrv_config(2, (uint32) local_ip, (uint32) gateway, 0);
    WiFiDrv_setDNS(1, (uint32) dns_server, 0);
}

void
WiFi_config_static_dns_gateway_subnet(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet) {
    WiFiDrv_config(3, (uint32) local_ip, (uint32) gateway, (uint32) subnet);
    WiFiDrv_setDNS(1, (uint32) dns_server, 0);
}

void WiFi_setDNS(IPAddress dns_server1) {
    WiFiDrv_setDNS(1, (uint32) dns_server1, 0);
}

void WiFi_setDNS_two_servers(IPAddress dns_server1, IPAddress dns_server2) {
    WiFiDrv_setDNS(2, (uint32) dns_server1, (uint32) dns_server2);
}

void WiFi_setHostname(const char *name) {
    WiFiDrv_setHostname(name);
}

int WiFi_disconnect() {
    return WiFiDrv_disconnect();
}

void WiFi_end(void) {
    WiFiDrv_wifiDriverDeinit();
}

uint8 *WiFi_macAddress(uint8 *mac) {
    uint8 *_mac = WiFiDrv_getMacAddress();
    memcpy(mac, _mac, WL_MAC_ADDR_LENGTH);
    return mac;
}

IPAddress WiFi_localIP() {
    IPAddress ret;
    WiFiDrv_getIpAddress(ret);
    return ret;
}

IPAddress WiFi_subnetMask() {
    IPAddress ret;
    WiFiDrv_getSubnetMask(ret);
    return ret;
}

IPAddress WiFi_gatewayIP() {
    IPAddress ret;
    WiFiDrv_getGatewayIP(ret);
    return ret;
}

const char *WiFi_SSID() {
    return WiFiDrv_getCurrentSSID();
}

uint8 *WiFi_BSSID(uint8 *bssid) {
    uint8 *_bssid = WiFiDrv_getCurrentBSSID();
    memcpy(bssid, _bssid, WL_MAC_ADDR_LENGTH);
    return bssid;
}

int32 WiFi_RSSI() {
    return WiFiDrv_getCurrentRSSI();
}

uint8 WiFi_encryptionType() {
    return WiFiDrv_getCurrentEncryptionType();
}


int8 WiFi_scanNetworks() {
    uint8 attempts = 10;
    uint8 numOfNetworks = 0;

    if (WiFiDrv_startScanNetworks() == WL_FAILURE) {
        return WL_FAILURE;
    }

    do {
        delay(2000);
        numOfNetworks = WiFiDrv_getScanNetworks();
    } while ((numOfNetworks == 0) && (--attempts > 0));
    return numOfNetworks;
}

const char *WiFi_SSID_index(uint8 networkItem) {
    return WiFiDrv_getSSIDNetworks(networkItem);
}

int32 WiFi_RSSI_index(uint8 networkItem) {
    return WiFiDrv_getRSSINetworks(networkItem);
}

uint8 WiFi_encryptionType_index(uint8 networkItem) {
    return WiFiDrv_getEncTypeNetworks(networkItem);
}

uint8 *WiFi_BSSID_index(uint8 networkItem, uint8 *bssid) {
    return WiFiDrv_getBSSIDNetworks(networkItem, bssid);
}

uint8 WiFi_channelIndex(uint8 networkItem) {
    return WiFiDrv_getChannelNetworks(networkItem);
}

uint8 WiFi_status() {
    return WiFiDrv_getConnectionStatus();
}

int WiFi_hostByName(const char *aHostname, IPAddress *aResult) {
    return WiFiDrv_getHostByName(aHostname, aResult);
}

unsigned long WiFi_getTime() {
    return WiFiDrv_getTime();
}

void WiFi_lowPowerMode() {
    WiFiDrv_setPowerMode(1);
}

void WiFi_noLowPowerMode() {
    WiFiDrv_setPowerMode(0);
}

int WiFi_ping_hostname(const char *hostname, int ttl) {
    IPAddress ip;

    if (!hostByName(hostname, ip)) {
        return WL_PING_UNKNOWN_HOST;
    }

    return WiFi_ping_ipaddress(ip, ttl);
}

int WiFi_ping_ipaddress(IPAddress host, int ttl) {
    if (ttl <= 0) {
        ttl = 128;
    }
    return WiFiDrv_ping(host, ttl);
}
