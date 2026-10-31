#include "settings.h"
#include "sensor.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_sleep.h>
RTC_DATA_ATTR uint32_t bootCount=0;
void setup() {
    Serial.begin(115200); ++bootCount;
    Wire.begin(21,22); Wire.setTimeOut(50); Reading reading=readSht30();
    WiFi.mode(WIFI_STA); WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
    uint32_t started=millis();
    while(strlen(WIFI_SSID) && WiFi.status()!=WL_CONNECTED && uint32_t(millis()-started)<8000) delay(50);
    WiFiClient transport; PubSubClient mqtt(transport); mqtt.setServer(MQTT_HOST,MQTT_PORT); mqtt.setSocketTimeout(2);
    String id="battery-"+String(uint32_t(ESP.getEfuseMac()),HEX);
    char payload[220];
    if(reading.valid) snprintf(payload,sizeof(payload),"{\"seq\":%lu,\"uptime_ms\":%lu,\"valid\":true,\"temperature_c\":%.2f,\"humidity_pct\":%.2f}",(unsigned long)bootCount,(unsigned long)millis(),reading.temperature,reading.humidity);
    else snprintf(payload,sizeof(payload),"{\"seq\":%lu,\"valid\":false,\"temperature_c\":null,\"humidity_pct\":null}",(unsigned long)bootCount);
    Serial.println(payload);
    bool sent=false;
    if(WiFi.status()==WL_CONNECTED && mqtt.connect(id.c_str(),MQTT_USER,MQTT_PASSWORD)) {
        sent=mqtt.publish("handbook/battery/telemetry",payload,false);
        mqtt.loop(); mqtt.disconnect(); // QoS0 does not prove application receipt
    }
    Serial.printf("publish_local_result=%d active_ms=%lu wake_reason=%d\n",sent,(unsigned long)millis(),int(esp_sleep_get_wakeup_cause()));
    WiFi.disconnect(true); WiFi.mode(WIFI_OFF);
    esp_sleep_enable_timer_wakeup(60ULL*1000000ULL);
    Serial.flush(); esp_deep_sleep_start();
}
void loop() {}
