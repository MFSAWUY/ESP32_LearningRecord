#include <Arduino.h>
#include <Wire.h>
constexpr uint8_t SENSOR=0x44;
uint8_t crc8(const uint8_t *data,size_t n){
  uint8_t crc=0xff;
  for(size_t i=0;i<n;i++){crc^=data[i];for(unsigned bit=0;bit<8;bit++)crc=(crc&0x80)?uint8_t((crc<<1)^0x31):uint8_t(crc<<1);}
  return crc;
}
bool measure(float &t,float &rh){
  Wire.beginTransmission(SENSOR);Wire.write(0x24);Wire.write(0x00);
  if(Wire.endTransmission()!=0)return false;
  delay(20); // 数据手册要求的最大转换时间留余量，主线为低频采样。
  if(Wire.requestFrom(SENSOR,uint8_t(6))!=6)return false;
  uint8_t data[6];for(auto &b:data)b=Wire.read();
  if(crc8(data,2)!=data[2]||crc8(data+3,2)!=data[5])return false;
  uint16_t rawT=uint16_t(data[0])<<8|data[1], rawRh=uint16_t(data[3])<<8|data[4];
  t=-45.0f+175.0f*rawT/65535.0f;rh=100.0f*rawRh/65535.0f;return true;
}
void setup(){Serial.begin(115200);Wire.begin(21,22);Wire.setClock(100000);Wire.setTimeOut(50);}
void loop(){
  float t,rh;
  if(measure(t,rh))Serial.printf("{\"valid\":true,\"temperature_c\":%.2f,\"humidity_pct\":%.2f}\n",t,rh);
  else Serial.println("{\"valid\":false,\"error\":\"sensor_read_or_crc\"}");
  delay(1000);
}
