#include <Arduino.h>
constexpr uint8_t SIGNAL=25;
bool ready=false;
void setup(){Serial.begin(115200);ready=ledcAttach(SIGNAL,50,16);if(ready)ledcWrite(SIGNAL,0);Serial.println("Send 0/1/2 for 1000/1500/2000 us; x disables; auto-off after 3s");}
void loop(){
  static uint32_t lastCommand=0;static bool active=false;
  if(!ready){delay(100);return;}
  while(Serial.available()){
    char c=Serial.read();
    if(c=='x'){ledcWrite(SIGNAL,0);active=false;}
    else if(c>='0'&&c<='2'){
      uint32_t us=1000+(c-'0')*500;
      uint32_t duty=uint64_t(us)*65536/20000;
      ledcWrite(SIGNAL,duty);lastCommand=millis();active=true;
    }
  }
  if(active && millis()-lastCommand>3000){ledcWrite(SIGNAL,0);active=false;Serial.println("signal timeout: disabled");}
  delay(1);
}
