#include "app_network.h"
#include <WiFiUdp.h>
WiFiServer server(5000);WiFiClient peer;WiFiUDP udp;
bool started=false;uint32_t activity=0;
void setup(){Serial.begin(115200);startNetwork();}
void loop(){
  if(!serviceNetwork()){peer.stop();if(started){server.end();udp.stop();started=false;}delay(10);return;}
  if(!started){server.begin();udp.begin(5001);started=true;Serial.println("TCP5000 UDP5001 echo ready");}
  if(!peer||!peer.connected()){peer=server.accept();activity=millis();}
  if(peer && peer.available()){
    uint8_t data[128];int n=peer.read(data,sizeof(data));
    if(n>0){peer.write(data,n);activity=millis();}
  }
  if(peer && millis()-activity>5000)peer.stop();
  int packet=udp.parsePacket();
  if(packet){uint8_t data[128];int n=udp.read(data,sizeof(data));
    if(packet<=128&&n>0){udp.beginPacket(udp.remoteIP(),udp.remotePort());udp.write(data,n);udp.endPacket();}
    else {while(udp.available())udp.read();Serial.println("oversize UDP dropped");}}
  delay(1);
}
