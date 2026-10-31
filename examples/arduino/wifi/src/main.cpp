#include "app_network.h"
void setup(){Serial.begin(115200);startNetwork();}
void loop(){static uint32_t last=0;bool connected=serviceNetwork();if(millis()-last>=3000){last=millis();Serial.printf("connected=%d rssi_dbm=%ld\n",connected,connected?WiFi.RSSI():0L);}delay(10);}
