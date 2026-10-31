#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
bool mounted=false;
void setup(){
  Serial.begin(115200);SPI.begin(18,19,23,13);mounted=SD.begin(13,SPI,4000000);
  if(!mounted){Serial.println("SD mount failed; check 3.3V-compatible module and FAT card");return;}
  if(!SD.exists("/log.csv")){File f=SD.open("/log.csv",FILE_WRITE);if(f){f.println("time_ms,heap_bytes");f.close();}}
}
void loop(){
  if(!mounted){delay(1000);return;}
  File f=SD.open("/log.csv",FILE_APPEND);
  if(!f)Serial.println("open failed");
  else {size_t written=f.printf("%u,%u\n",unsigned(millis()),unsigned(ESP.getFreeHeap()));f.flush();f.close();Serial.printf("written=%u\n",unsigned(written));}
  delay(1000);
}
