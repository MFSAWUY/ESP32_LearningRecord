#pragma once
#include "settings.h"
#include <WiFi.h>
uint32_t lastWifiTry = 0;
void startWifi() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    if (strlen(WIFI_SSID)) WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    lastWifiTry = millis();
}
void pollWifi() {
    if (strlen(WIFI_SSID) && WiFi.status()!=WL_CONNECTED && uint32_t(millis()-lastWifiTry)>=15000) {
        lastWifiTry=millis(); WiFi.disconnect(); WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
    }
}
