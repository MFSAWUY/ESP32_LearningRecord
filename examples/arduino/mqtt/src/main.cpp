#include "app_network.h"
#include <PubSubClient.h>
WiFiClient transport;PubSubClient mqtt(transport);
String telemetryTopic,commandTopic,ackTopic;
void received(char *topic,byte *payload,unsigned length){
  if(String(topic)!=commandTopic || length>16)return;
  // 幂等命令，重复ON不会翻转输出；未知内容不会驱动负载。
  if(length==2 && memcmp(payload,"ON",2)==0){digitalWrite(25,HIGH);mqtt.publish(ackTopic.c_str(),"ON");}
  else if(length==3 && memcmp(payload,"OFF",3)==0){digitalWrite(25,LOW);mqtt.publish(ackTopic.c_str(),"OFF");}
  else mqtt.publish(ackTopic.c_str(),"INVALID");
}
void setup(){
  Serial.begin(115200);pinMode(25,OUTPUT);digitalWrite(25,LOW);startNetwork();
  String base=String("lab/")+DEVICE_ID;telemetryTopic=base+"/telemetry";commandTopic=base+"/command";ackTopic=base+"/ack";
  mqtt.setServer(MQTT_HOST,MQTT_PORT);mqtt.setCallback(received);mqtt.setSocketTimeout(3);mqtt.setBufferSize(256);
}
void loop(){
  static uint32_t attempt=0,published=0;
  if(!serviceNetwork()){digitalWrite(25,LOW);mqtt.disconnect();delay(10);return;}
  if(!mqtt.connected()){
    digitalWrite(25,LOW);
    if(millis()-attempt>=5000){attempt=millis();
      if(mqtt.connect(DEVICE_ID,MQTT_USER,MQTT_PASSWORD))mqtt.subscribe(commandTopic.c_str(),1);
      else Serial.printf("mqtt state=%d\n",mqtt.state());}
  }else{
    mqtt.loop();
    if(millis()-published>=2000){published=millis();char payload[96];snprintf(payload,sizeof(payload),"{\"uptime_ms\":%u,\"heap\":%u}",unsigned(millis()),unsigned(ESP.getFreeHeap()));Serial.printf("publish=%d\n",mqtt.publish(telemetryTopic.c_str(),payload));}
  }
  delay(5);
}
