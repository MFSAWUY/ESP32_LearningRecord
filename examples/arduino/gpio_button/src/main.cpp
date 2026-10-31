#include <Arduino.h>
constexpr uint8_t LED=25, BUTTON=27;
bool stable=HIGH, previous=HIGH, on=false;
uint32_t changed=0, presses=0;
void setup(){ Serial.begin(115200); pinMode(LED,OUTPUT); digitalWrite(LED,LOW); pinMode(BUTTON,INPUT_PULLUP); }
void loop(){
  uint32_t now=millis(); bool raw=digitalRead(BUTTON);
  if(raw!=previous){previous=raw;changed=now;}
  // 无符号减法可跨 millis() 回绕；要求检查间隔远小于整个计数周期。
  if(now-changed>=30 && stable!=raw){
    stable=raw;
    if(stable==LOW){on=!on;digitalWrite(LED,on);Serial.printf("press=%u led=%d\n",unsigned(++presses),on);}
  }
  delay(1);
}
