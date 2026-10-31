#include "app_network.h"
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
WebServer server(80);WebSocketsServer sockets(81);
bool started=false;
// TLS测试仅在提供真实CA和匹配域名后启用，绝不调用setInsecure。
#if __has_include("tls_config.h")
#include "tls_config.h"
#endif
void setup(){
  Serial.begin(115200);startNetwork();
  server.on("/status",HTTP_GET,[]{server.send(200,"application/json",String("{\"uptime_ms\":")+millis()+"}");});
  server.onNotFound([]{server.send(404,"text/plain","not found");});
  sockets.onEvent([](uint8_t id,WStype_t type,uint8_t *payload,size_t length){
    if(type==WStype_TEXT && length<=128)sockets.sendTXT(id,payload,length);
    else if(type==WStype_TEXT)sockets.disconnect(id);
  });
}
void loop(){
  if(!serviceNetwork()){delay(10);return;}
  if(!started){server.begin();sockets.begin();configTime(0,0,"pool.ntp.org");started=true;Serial.println("HTTP80 /status; WebSocket81 echo");}
  server.handleClient();sockets.loop();
#ifdef HTTPS_URL
  static bool requested=false;
  if(!requested&&time(nullptr)>1700000000){
    requested=true;WiFiClientSecure tls;tls.setCACert(ROOT_CA);tls.setTimeout(5000);
    HTTPClient http;http.setConnectTimeout(5000);http.setTimeout(5000);
    if(http.begin(tls,HTTPS_URL)){int status=http.GET();Serial.printf("https_status=%d\n",status);http.end();}
  }
#endif
  delay(1);
}
