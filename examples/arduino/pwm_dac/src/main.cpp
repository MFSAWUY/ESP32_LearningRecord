#include <Arduino.h>
constexpr uint8_t PWM_PIN=25, DAC_PIN=26;
bool ready=false;
void setup(){Serial.begin(115200);ready=ledcAttach(PWM_PIN,5000,8);if(!ready) Serial.println("LEDC attach failed");}
void loop(){
  static int value=0,step=1; static uint32_t last=0;
  if(!ready){delay(100);return;}
  if(millis()-last<10){delay(1);return;} last=millis();
  ledcWrite(PWM_PIN,value); dacWrite(DAC_PIN,value);
  value+=step; if(value==255 || value==0) step=-step;
}
