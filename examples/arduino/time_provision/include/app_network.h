#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
inline void startNetwork(){WiFi.persistent(false);WiFi.mode(WIFI_STA);WiFi.setAutoReconnect(true);WiFi.begin(WIFI_SSID,WIFI_PASSWORD);}
inline bool serviceNetwork(){
  static uint32_t lastAttempt=0;static bool announced=false;
  if(WiFi.status()==WL_CONNECTED){
    if(!announced){Serial.print("IP=");Serial.println(WiFi.localIP());announced=true;}
    return true;
  }
  announced=false;
  if(millis()-lastAttempt>=15000){lastAttempt=millis();Serial.println("wifi retry");WiFi.disconnect();WiFi.begin(WIFI_SSID,WIFI_PASSWORD);}
  return false;
}
