#include <Arduino.h>
constexpr uint8_t ADC_PIN=34;
void setup(){Serial.begin(115200);analogReadResolution(12);analogSetPinAttenuation(ADC_PIN,ADC_11db);}
void loop(){
  uint32_t rawSum=0,mvSum=0;
  for(unsigned i=0;i<16;i++){
    rawSum+=analogRead(ADC_PIN);
    mvSum+=analogReadMilliVolts(ADC_PIN); // 独立一次采样，经框架校准路径转换
    delay(2);
  }
  Serial.printf("{\"raw\":%.2f,\"mv\":%.2f,\"time_ms\":%u}\n",rawSum/16.0,mvSum/16.0,unsigned(millis()));
  delay(200);
}
