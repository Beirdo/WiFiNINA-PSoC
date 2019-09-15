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

#include "wl_definitions.h"
#include "wl_types.h"

#include "FreeRTOS.h"
#include "task.h"

void WiFi_setLEDs(uint8 red, uint8 green, uint8 blue) {
    WiFiDrv_pinMode(25, 1);  // OUTPUT
    WiFiDrv_pinMode(26, 1);
    WiFiDrv_pinMode(27, 1);
    WiFiDrv_analogWrite(25, red);
    WiFiDrv_analogWrite(26, green);
    WiFiDrv_analogWrite(27, blue);
}

void WiFi_init() {
    WiFiDrv_wifiDriverInit();
}

uint8 *WiFi_firmwareVersion() {
    return WiFiDrv_getFwVersion();
}

int WiFi_begin_common(void) {
    uint8 status = WL_IDLE_STATUS;
    uint8 attempts = WL_MAX_ATTEMPT_CONNECTION;

    do {
        vTaskDelay(pdMS_TO_TICKS(WL_DELAY_START_CONNECTION));
        status = WiFiDrv_getConnectionStatus();
    } while (((status == WL_IDLE_STATUS) || (status == WL_NO_SSID_AVAIL) || (status == WL_SCAN_COMPLETED)) &&
             (--attempts > 0));
    return status;

}

int WiFi_begin_open(uint8 *ssid) {
    if (WiFiDrv_wifiSetNetwork(ssid, ustrlen(ssid)) == WL_FAILURE) {
        return WL_CONNECT_FAILED;
    }
    return WiFi_begin_common();
}

int WiFi_begin_WEP(uint8 *ssid, uint8 key_idx, uint8 *key) {
    // set encryption key
    if (WiFiDrv_wifiSetKey(ssid, ustrlen(ssid), key_idx, key, ustrlen(key)) == WL_FAILURE) {
        return WL_CONNECT_FAILED;
    }
    return WiFi_begin_common();
}

int WiFi_begin_passphrase(uint8 *ssid, uint8 *passphrase) {
    // set passphrase
    if (WiFiDrv_wifiSetPassphrase(ssid, ustrlen(ssid), passphrase, ustrlen(passphrase)) == WL_FAILURE) {
        return WL_CONNECT_FAILED;
    }
    return WiFi_begin_common();
}

uint8 WiFi_beginAP_common(void) {
    uint8 status = WL_IDLE_STATUS;
    uint8 attempts = WL_MAX_ATTEMPT_CONNECTION;

    do {
        vTaskDelay(pdMS_TO_TICKS(WL_DELAY_START_CONNECTION));
        status = WiFiDrv_getConnectionStatus();
    } while (((status == WL_IDLE_STATUS) || (status == WL_SCAN_COMPLETED)) && (--attempts > 0));
    return status;
}

uint8 WiFi_beginAP_ssid(uint8 *ssid) {
    return WiFi_beginAP_ssid_channel(ssid, 1);
}

uint8 WiFi_beginAP_ssid_channel(uint8 *ssid, uint8 channel) {
    if (WiFiDrv_wifiSetApNetwork(ssid, ustrlen(ssid), channel) == WL_FAILURE) {
        return WL_AP_FAILED;
    }
    return WiFi_beginAP_common();
}

uint8 WiFi_beginAP_ssid_passphrase(uint8 *ssid, uint8 *passphrase) {
    return WiFi_beginAP_ssid_passphrase_channel(ssid, passphrase, 1);
}

uint8 WiFi_beginAP_ssid_passphrase_channel(uint8 *ssid, uint8 *passphrase, uint8 channel) {
    // set passphrase
    if (WiFiDrv_wifiSetApPassphrase(ssid, ustrlen(ssid), passphrase, ustrlen(passphrase), channel) == WL_FAILURE) {
        return WL_AP_FAILED;
    }
    return WiFi_beginAP_common();
}

void WiFi_config_static(uint32 local_ip) {
    WiFiDrv_config(1, (uint32) local_ip, 0, 0);
}

void WiFi_config_static_dns(uint32 local_ip, uint32 dns_server) {
    WiFiDrv_config(1, (uint32) local_ip, 0, 0);
    WiFiDrv_setDNS(1, (uint32) dns_server, 0);
}

void WiFi_config_static_dns_gateway(uint32 local_ip, uint32 dns_server, uint32 gateway) {
    WiFiDrv_config(2, (uint32) local_ip, (uint32) gateway, 0);
    WiFiDrv_setDNS(1, (uint32) dns_server, 0);
}

void
WiFi_config_static_dns_gateway_subnet(uint32 local_ip, uint32 dns_server, uint32 gateway, uint32 subnet) {
    WiFiDrv_config(3, (uint32) local_ip, (uint32) gateway, (uint32) subnet);
    WiFiDrv_setDNS(1, (uint32) dns_server, 0);
}

void WiFi_setDNS(uint32 dns_server1) {
    WiFiDrv_setDNS(1, (uint32) dns_server1, 0);
}

void WiFi_setDNS_two_servers(uint32 dns_server1, uint32 dns_server2) {
    WiFiDrv_setDNS(2, (uint32) dns_server1, (uint32) dns_server2);
}

void WiFi_setHostname(uint8 *name) {
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

uint32 WiFi_localIP() {
    uint32 ret;
    WiFiDrv_getIpAddress(&ret);
    return ret;
}

uint32 WiFi_subnetMask() {
    uint32 ret;
    WiFiDrv_getSubnetMask(&ret);
    return ret;
}

uint32 WiFi_gatewayIP() {
    uint32 ret;
    WiFiDrv_getGatewayIP(&ret);
    return ret;
}

uint8 *WiFi_SSID() {
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
        vTaskDelay(pdMS_TO_TICKS(2000));
        numOfNetworks = WiFiDrv_getScanNetworks();
    } while ((numOfNetworks == 0) && (--attempts > 0));
    return numOfNetworks;
}

uint8 *WiFi_SSID_index(uint8 networkItem) {
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

uint8 WiFi_channel_index(uint8 networkItem) {
    return WiFiDrv_getChannelNetworks(networkItem);
}

uint8 WiFi_status() {
    return WiFiDrv_getConnectionStatus();
}

int WiFi_hostByName(uint8 *aHostname, uint32 *aResult) {
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

int WiFi_ping_hostname(uint8 *hostname, int ttl) {
    uint32 ip;

    if (!WiFi_hostByName(hostname, &ip)) {
        return WL_PING_UNKNOWN_HOST;
    }

    return WiFi_ping_ipaddress(ip, ttl);
}

int WiFi_ping_ipaddress(uint32 host, int ttl) {
    if (ttl <= 0) {
        ttl = 128;
    }
    return WiFiDrv_ping(host, ttl);
}
