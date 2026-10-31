#include "settings.h"
#include "display.h"
#include "wifi_support.h"
#include <time.h>
uint32_t lastRefresh=0;
bool ntpStarted=false;
void setup() {
    Serial.begin(115200); startDisplay(); startWifi();
    setenv("TZ", TZ_RULE, 1); tzset();
}
void loop() {
    pollWifi();
    if(WiFi.status()==WL_CONNECTED && !ntpStarted) {
        configTime(0,0,NTP_SERVER); setenv("TZ",TZ_RULE,1); tzset(); ntpStarted=true;
    }
    if(uint32_t(millis()-lastRefresh)>=1000) {
        lastRefresh=millis(); time_t now=time(nullptr); struct tm local{};
        bool timeValid=now>1704067200; char text[32]="Waiting for SNTP";
        if(timeValid) { localtime_r(&now,&local); strftime(text,sizeof(text),"%Y-%m-%d %H:%M:%S",&local); }
        Serial.printf("{\"uptime_ms\":%lu,\"valid\":%s,\"wifi\":%s,\"time\":\"%s\"}\n",
                      (unsigned long)millis(),timeValid?"true":"false",WiFi.status()==WL_CONNECTED?"true":"false",text);
        if(displayReady) { screen.clearBuffer(); screen.drawStr(0,12,text);
            screen.drawStr(0,27,WiFi.status()==WL_CONNECTED?"WiFi connected":"WiFi offline");
            if(OLED_HEIGHT==64) { screen.setCursor(0,45); screen.print("Uptime "); screen.print(millis()/1000); screen.print(" s"); }
            screen.sendBuffer(); }
    }
    delay(1);
}
