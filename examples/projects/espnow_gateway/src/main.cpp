#include "settings.h"
#include "sensor.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
// Explicit wire format: version, type(1=data/2=ack), seq little-endian,
// signed centidegrees little-endian. No transmitted raw C struct/padding.
struct Received { uint8_t mac[6]; uint8_t bytes[8]; };
QueueHandle_t incoming;
uint32_t sequence=0,lastSend=0,lastPending=0;
bool waiting=false;
bool ready=false;
const uint8_t broadcastAddress[6]={255,255,255,255,255,255};
uint32_t decodeSeq(const uint8_t* data){return uint32_t(data[2])|(uint32_t(data[3])<<8)|(uint32_t(data[4])<<16)|(uint32_t(data[5])<<24);}
void received(const esp_now_recv_info_t* info,const uint8_t* data,int length){
    if(!info || !data || length!=8 || data[0]!=1) return;
    Received message; memcpy(message.mac,info->src_addr,6); memcpy(message.bytes,data,8);
    xQueueSend(incoming,&message,0); // Wi-Fi task callback, not an ISR; no blocking
}
void setup(){
    Serial.begin(115200); WiFi.mode(WIFI_STA);
    Wire.begin(21,22); Wire.setTimeOut(50);
    if(esp_wifi_set_channel(ESPNOW_CHANNEL,WIFI_SECOND_CHAN_NONE)!=ESP_OK){Serial.println("channel error");return;}
    incoming=xQueueCreate(8,sizeof(Received));
    if(!incoming || esp_now_init()!=ESP_OK){Serial.println("ESP-NOW init error");return;}
    esp_now_peer_info_t peer{};memcpy(peer.peer_addr,broadcastAddress,6);peer.channel=ESPNOW_CHANNEL;peer.encrypt=false;
    if(esp_now_add_peer(&peer)!=ESP_OK){Serial.println("peer error");return;}
    esp_now_register_recv_cb(received);
    ready=true;
    Serial.printf("MAC=%s channel=%d role=%s\n",WiFi.macAddress().c_str(),ESPNOW_CHANNEL,
#ifdef NODE_ROLE
    "node"
#else
    "gateway"
#endif
    );
}
void loop(){
    if(!ready){delay(1000);return;}
#ifdef NODE_ROLE
    if(uint32_t(millis()-lastSend)>=3000){
        lastSend=millis();++sequence;uint8_t frame[8]={1,1};
        for(int index=0;index<4;++index)frame[index+2]=(sequence>>(8*index))&0xff;
        Reading reading=readSht30();
        int16_t temperature=reading.valid?int16_t(lroundf(reading.temperature*100.0f)):INT16_MIN;
        frame[6]=uint16_t(temperature)&0xff; frame[7]=(uint16_t(temperature)>>8)&0xff;
        if(esp_now_send(broadcastAddress,frame,8)==ESP_OK){waiting=true;lastPending=millis();}
    }
    if(waiting && uint32_t(millis()-lastPending)>500){Serial.println("application ACK timeout; no automatic retry in this exercise");waiting=false;}
#endif
    Received message;
    while(xQueueReceive(incoming,&message,0)==pdTRUE){
        uint32_t seq=decodeSeq(message.bytes);
#ifdef NODE_ROLE
        if(message.bytes[1]==2 && seq==sequence && waiting){waiting=false;Serial.printf("ACK seq=%lu rtt_ms=%lu\n",(unsigned long)seq,(unsigned long)(millis()-lastPending));}
#else
        if(message.bytes[1]!=1)continue;
        int16_t temperature=int16_t(uint16_t(message.bytes[6])|(uint16_t(message.bytes[7])<<8));
        char value[24]; if(temperature==INT16_MIN)snprintf(value,sizeof(value),"null");else snprintf(value,sizeof(value),"%.2f",temperature/100.0f);
        Serial.printf("{\"node\":\"%02X:%02X:%02X:%02X:%02X:%02X\",\"seq\":%lu,\"valid\":%s,\"temperature_c\":%s}\n",message.mac[0],message.mac[1],message.mac[2],message.mac[3],message.mac[4],message.mac[5],(unsigned long)seq,temperature==INT16_MIN?"false":"true",value);
        if(!esp_now_is_peer_exist(message.mac)) { esp_now_peer_info_t peer{};memcpy(peer.peer_addr,message.mac,6);peer.channel=ESPNOW_CHANNEL;peer.encrypt=false;
            if(esp_now_add_peer(&peer)!=ESP_OK){Serial.println("peer table full; ACK skipped");continue;} }
        message.bytes[1]=2;
        if(esp_now_send(message.mac,message.bytes,8)!=ESP_OK)Serial.println("ACK enqueue failed");
#endif
    }
    delay(1);
}
