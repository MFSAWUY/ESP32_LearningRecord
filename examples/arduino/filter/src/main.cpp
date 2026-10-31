#include <Arduino.h>
constexpr size_t N=8;
uint32_t samples[N]={};uint32_t sum=0;size_t used=0,sampleIndex=0;float lowpass=0;
void setup(){Serial.begin(115200);analogSetPinAttenuation(34,ADC_11db);}
void loop(){
  static uint32_t last=0;uint32_t now=millis();
  if(now-last<20){delay(1);return;}
  float dt=(now-last)*0.001f;last=now;
  uint32_t value=analogReadMilliVolts(34);
  sum-=samples[sampleIndex];samples[sampleIndex]=value;sum+=value;sampleIndex=(sampleIndex+1)%N;
  if(used<N)used++;
  if(used==1)lowpass=value;
  float alpha=dt/(0.1f+dt); // 后向欧拉离散的一阶低通，tau=0.1 s
  lowpass+=alpha*(value-lowpass);
  Serial.printf("%u,%.2f,%.2f\n",unsigned(value),float(sum)/used,lowpass);
}
