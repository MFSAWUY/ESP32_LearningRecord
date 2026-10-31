#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
SemaphoreHandle_t ticks=nullptr;
hw_timer_t *timer=nullptr;
void ARDUINO_ISR_ATTR onTick(){
  BaseType_t wake=pdFALSE;
  xSemaphoreGiveFromISR(ticks,&wake);
  if(wake) portYIELD_FROM_ISR();
}
void setup(){
  Serial.begin(115200); ticks=xSemaphoreCreateCounting(10,0);
  if(!ticks){Serial.println("semaphore allocation failed");return;}
  timer=timerBegin(1000000); // 每计数一次为 1 us
  if(!timer){Serial.println("timer failed");return;}
  timerAttachInterrupt(timer,onTick);
  timerAlarm(timer,1000000,true,0); // 每秒产生一次中断
}
void loop(){
  static uint32_t count=0;
  if(ticks && xSemaphoreTake(ticks,pdMS_TO_TICKS(100))==pdTRUE)
    Serial.printf("tick=%u time_ms=%u\n",unsigned(++count),unsigned(millis()));
  else delay(1);
}
