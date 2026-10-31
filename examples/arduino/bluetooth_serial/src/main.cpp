#include <Arduino.h>
#include <BluetoothSerial.h>
#if !defined(CONFIG_BT_SPP_ENABLED)
#error "This example requires classic ESP32 with SPP enabled."
#endif
BluetoothSerial bt;
void setup(){Serial.begin(115200);if(!bt.begin("ESP32-Handbook-SPP"))Serial.println("SPP failed");}
void loop(){
  // 有界每轮最多处理64字节，避免单侧连续输入饿死另一方向。
  for(unsigned i=0;i<64&&bt.available();i++){int c=bt.read();Serial.write(c);bt.write(c);}
  for(unsigned i=0;i<64&&Serial.available();i++)bt.write(Serial.read());
  delay(1);
}
