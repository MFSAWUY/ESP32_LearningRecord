#include <Arduino.h>
#include <driver/pulse_cnt.h>
pcnt_unit_handle_t counter=nullptr;
pcnt_channel_handle_t input=nullptr;
bool ready=false;
void setup(){
  Serial.begin(115200);
  pcnt_unit_config_t unit={};unit.high_limit=30000;unit.low_limit=-30000;
  if(pcnt_new_unit(&unit,&counter)!=ESP_OK)return;
  pcnt_chan_config_t ch={};ch.edge_gpio_num=27;ch.level_gpio_num=-1;
  if(pcnt_new_channel(counter,&ch,&input)!=ESP_OK)return;
  if(pcnt_channel_set_edge_action(input,PCNT_CHANNEL_EDGE_ACTION_INCREASE,PCNT_CHANNEL_EDGE_ACTION_HOLD)!=ESP_OK)return;
  if(pcnt_unit_enable(counter)!=ESP_OK)return;
  if(pcnt_unit_clear_count(counter)!=ESP_OK)return;
  if(pcnt_unit_start(counter)!=ESP_OK)return;
  ready=rmtInit(25,RMT_TX_MODE,RMT_MEM_NUM_BLOCKS_1,1000000);
  Serial.println("Connect GPIO25 to GPIO27; touch electrode on GPIO4");
}
void loop(){
  if(!ready){Serial.println("peripheral init failed");delay(1000);return;}
  rmt_data_t pulse[10]={};
  for(auto &p:pulse){p.level0=1;p.duration0=500;p.level1=0;p.duration1=500;}
  if(!rmtWrite(25,pulse,10,100))Serial.println("RMT timeout");
  int count=0;if(pcnt_unit_get_count(counter,&count)==ESP_OK)Serial.printf("pulses=%d touch=%u\n",count,unsigned(touchRead(4)));
  // 本实验每秒只发送10脉冲；每轮清零，避免16位计数器长时溢出。
  pcnt_unit_clear_count(counter);delay(1000);
}
