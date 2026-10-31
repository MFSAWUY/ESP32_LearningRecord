#include <Arduino.h>
HardwareSerial uartLink(2);
void setup(){ Serial.begin(115200);uartLink.begin(9600,SERIAL_8N1,16,17);Serial.println("Connect GPIO17 TX to GPIO16 RX for loopback"); }
void loop(){
  static uint32_t last=0,seq=0; static char line[64];static size_t used=0;static bool overflow=false;
  if(millis()-last>=1000){last=millis();uartLink.printf("PING %u\n",unsigned(seq++));}
  while(uartLink.available()){
    char c=uartLink.read();if(c=='\r')continue;
    if(c=='\n'){
      line[used]=0;Serial.printf("%s%s\n",overflow?"RX overflow: ":"RX: ",line);
      used=0;overflow=false;
    }else if(used<sizeof(line)-1)line[used++]=c;else overflow=true;
  }
  delay(1);
}
