#include "settings.h"
#include "display.h"
#include "sensor.h"
#include "wifi_support.h"
#include <PubSubClient.h>
#include <LittleFS.h>
WiFiClient transport;
PubSubClient mqtt(transport);
uint32_t lastSample=0,lastMqttTry=0,sequence=0;
bool fsReady=false;
void setup() {
    Serial.begin(115200); startDisplay(); startWifi();
    fsReady=LittleFS.begin(false); // never silently erase a filesystem
    if(!fsReady) Serial.println("LittleFS unavailable: serial/MQTT logging remains enabled");
    mqtt.setServer(MQTT_HOST,MQTT_PORT); mqtt.setSocketTimeout(2); mqtt.setBufferSize(384);
}
void loop() {
    pollWifi();
    if(WiFi.status()==WL_CONNECTED && !mqtt.connected() && uint32_t(millis()-lastMqttTry)>=10000) {
        lastMqttTry=millis(); String id="env-"+String(uint32_t(ESP.getEfuseMac()),HEX);
        mqtt.connect(id.c_str(),MQTT_USER,MQTT_PASSWORD);
    }
    mqtt.loop();
    if(uint32_t(millis()-lastSample)>=2000) {
        lastSample=millis(); Reading reading=readSht30(); char payload[220];
        if(reading.valid) snprintf(payload,sizeof(payload),"{\"seq\":%lu,\"uptime_ms\":%lu,\"valid\":true,\"temperature_c\":%.2f,\"humidity_pct\":%.2f}",
            (unsigned long)sequence++,(unsigned long)millis(),reading.temperature,reading.humidity);
        else snprintf(payload,sizeof(payload),"{\"seq\":%lu,\"uptime_ms\":%lu,\"valid\":false,\"temperature_c\":null,\"humidity_pct\":null}",(unsigned long)sequence++,(unsigned long)millis());
        Serial.println(payload);
        if(mqtt.connected() && !mqtt.publish("handbook/environment/telemetry",payload,false)) Serial.println("MQTT publish failed");
        // Bounded demo log: rotate at 64 KiB. Not a power-fail atomic database.
        if(fsReady) { File old=LittleFS.open("/telemetry.jsonl","r"); size_t size=old?old.size():0; old.close();
            if(size>65536) { LittleFS.remove("/previous.jsonl"); LittleFS.rename("/telemetry.jsonl","/previous.jsonl"); }
            File file=LittleFS.open("/telemetry.jsonl","a"); if(file) {file.println(payload); file.close();} }
        if(displayReady) { screen.clearBuffer(); screen.setCursor(0,12);
            if(reading.valid){screen.print("Temp ");screen.print(reading.temperature,1);screen.print(" C");screen.setCursor(0,27);screen.print("RH ");screen.print(reading.humidity,1);screen.print(" %");}
            else screen.drawStr(0,12,"Sensor missing/CRC");
            if(OLED_HEIGHT==64) screen.drawStr(0,45,mqtt.connected()?"MQTT online":"MQTT offline");
            screen.sendBuffer(); }
    }
    delay(1);
}
