#include <Arduino.h>
#include <Wire.h>
void setup(){Serial.begin(115200);Wire.begin(21,22);Wire.setClock(100000);Wire.setTimeOut(50);}
void loop(){
  unsigned found=0;
  for(uint8_t address=8;address<120;address++){
    Wire.beginTransmission(address);uint8_t result=Wire.endTransmission();
    if(result==0){Serial.printf("ACK 7-bit address=0x%02X\n",unsigned(address));found++;}
    else if(result!=2)Serial.printf("address=0x%02X status=%u\n",unsigned(address),unsigned(result));
  }
  Serial.printf("devices=%u (ACK is not chip identification)\n",found);delay(3000);
}
