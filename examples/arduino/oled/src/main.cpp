#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "config.h"
#if OLED_CONTROLLER == 1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0,U8X8_PIN_NONE,22,21);
#elif OLED_HEIGHT == 32
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C display(U8G2_R0,U8X8_PIN_NONE,22,21);
#else
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0,U8X8_PIN_NONE,22,21);
#endif
bool found=false;
void setup(){
  Serial.begin(115200);Wire.begin(21,22);Wire.setTimeOut(50);
  Wire.beginTransmission(OLED_ADDRESS);found=Wire.endTransmission()==0;
  if(!found){Serial.println("OLED no ACK; run i2c_scan, check wiring/address");return;}
  display.setI2CAddress(OLED_ADDRESS<<1); // U8g2 接收移位后的地址
  display.begin();display.setBusClock(100000);
}
void loop(){
  if(!found){delay(1000);return;}
  display.clearBuffer();display.setFont(u8g2_font_6x12_tf);
  display.drawStr(0,12,"ESP32 handbook");
  char text[32];snprintf(text,sizeof(text),"Uptime: %lu s",millis()/1000UL);
  display.drawStr(0,26,text);
  if(OLED_HEIGHT==64){display.drawFrame(0,36,128,16);display.drawBox(2,38,(millis()/100)%124,12);}
  display.sendBuffer();delay(100);
}
