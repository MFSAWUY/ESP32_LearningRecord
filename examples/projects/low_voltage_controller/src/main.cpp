#include "settings.h"
#include "wifi_support.h"
#include <WebServer.h>
WebServer server(80);
constexpr int OUTPUT_PIN=25;
bool outputOn=false;
uint32_t lastCommand=0;
constexpr uint32_t LEASE_MS=30000;
void setOutput(bool on) { outputOn=on; digitalWrite(OUTPUT_PIN,on?HIGH:LOW); }
bool authorized() {
    return strlen(CONTROL_TOKEN)>=16 && server.header("Authorization")==String("Bearer ")+CONTROL_TOKEN;
}
void setup() {
    Serial.begin(115200); pinMode(OUTPUT_PIN,OUTPUT); setOutput(false); startWifi();
    const char* headers[]={"Authorization"}; server.collectHeaders(headers,1);
    server.on("/api/status",HTTP_GET,[](){
        char json[128]; snprintf(json,sizeof(json),"{\"uptime_ms\":%lu,\"led\":%s,\"lease_ms\":%lu}",(unsigned long)millis(),outputOn?"true":"false",(unsigned long)LEASE_MS);
        server.send(200,"application/json",json);
    });
    server.on("/api/led",HTTP_POST,[](){
        if(!authorized()){server.send(401,"application/json","{\"error\":\"authorization required\"}");return;}
        String argument=server.arg("on");
        if(argument!="0" && argument!="1"){server.send(400,"application/json","{\"error\":\"on must be 0 or 1\"}");return;}
        setOutput(argument=="1"); lastCommand=millis(); server.send(200,"application/json","{\"accepted\":true}");
    });
    server.onNotFound([](){server.send(404,"application/json","{\"error\":\"not found\"}");});
    server.begin();
}
void loop() {
    pollWifi(); server.handleClient();
    if(outputOn && (WiFi.status()!=WL_CONNECTED || uint32_t(millis()-lastCommand)>=LEASE_MS)) setOutput(false);
    static uint32_t lastStatus=0;
    if(uint32_t(millis()-lastStatus)>=2000){lastStatus=millis();Serial.printf("IP=%s LED=%d\n",WiFi.localIP().toString().c_str(),outputOn);}
    delay(1);
}
