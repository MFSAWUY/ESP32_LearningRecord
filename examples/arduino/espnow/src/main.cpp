#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "config.h"
// 固定字节协议，不跨设备直接发送C结构体(填充和字节序可能不同)。
uint8_t broadcastMac[6]={255,255,255,255,255,255};
struct Rx{uint8_t mac[6];uint8_t bytes[7];};
QueueHandle_t incoming=nullptr;bool ready=false;
void receive(const esp_now_recv_info_t *info,const uint8_t *data,int len){
  if(len!=7||data[0]!=0xA5||data[1]!=1)return;
  Rx item;memcpy(item.mac,info->src_addr,6);memcpy(item.bytes,data,7);
  xQueueSend(incoming,&item,0); // Wi-Fi任务回调，非ISR；队列满时丢弃。
}
uint32_t number(const uint8_t *p){return uint32_t(p[3])<<24|uint32_t(p[4])<<16|uint32_t(p[5])<<8|p[6];}
void frame(uint8_t *p,uint8_t kind,uint32_t seq){p[0]=0xA5;p[1]=1;p[2]=kind;p[3]=seq>>24;p[4]=seq>>16;p[5]=seq>>8;p[6]=seq;}
bool addPeer(const uint8_t *mac){if(esp_now_is_peer_exist(mac))return true;esp_now_peer_info_t p={};memcpy(p.peer_addr,mac,6);p.channel=RADIO_CHANNEL;p.ifidx=WIFI_IF_STA;p.encrypt=false;return esp_now_add_peer(&p)==ESP_OK;}
void setup(){
  Serial.begin(115200);WiFi.mode(WIFI_STA);WiFi.disconnect();
  if(esp_wifi_set_channel(RADIO_CHANNEL,WIFI_SECOND_CHAN_NONE)!=ESP_OK)return;
  incoming=xQueueCreate(8,sizeof(Rx));if(!incoming||esp_now_init()!=ESP_OK)return;
  if(esp_now_register_recv_cb(receive)!=ESP_OK)return;
  ready=addPeer(broadcastMac);Serial.println(WiFi.macAddress());
}
void loop(){
  static uint32_t seq=0,sentAt=0;static bool waiting=false;
  if(!ready){Serial.println("ESP-NOW init failed");delay(1000);return;}
  Rx item;
  while(xQueueReceive(incoming,&item,0)==pdTRUE){
    uint32_t n=number(item.bytes);
    if(item.bytes[2]==0&&ROLE_SENDER==0){
      Serial.printf("received seq=%u\n",unsigned(n));
      if(addPeer(item.mac)){uint8_t ack[7];frame(ack,1,n);esp_now_send(item.mac,ack,7);}
    }else if(item.bytes[2]==1&&ROLE_SENDER==1&&waiting&&n==seq){Serial.printf("application ACK seq=%u rtt_ms=%u\n",unsigned(n),unsigned(millis()-sentAt));waiting=false;}
  }
  if(ROLE_SENDER==1){
    if(waiting&&millis()-sentAt>500){Serial.printf("ACK timeout seq=%u\n",unsigned(seq));waiting=false;}
    if(!waiting&&millis()-sentAt>=2000){uint8_t data[7];frame(data,0,++seq);sentAt=millis();waiting=esp_now_send(broadcastMac,data,7)==ESP_OK;}
  }
  delay(1);
}
