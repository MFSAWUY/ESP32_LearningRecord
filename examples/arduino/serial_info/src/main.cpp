// 原创教学示例：打印真实芯片信息，响应有界的串口命令。
#include <Arduino.h>
#include <esp_system.h>
void report() {
  Serial.printf("chip=%s cores=%d revision=%d cpu_mhz=%u\n", ESP.getChipModel(),
                ESP.getChipCores(), ESP.getChipRevision(), unsigned(ESP.getCpuFreqMHz()));
  Serial.printf("flash_bytes=%u heap_free=%u psram_bytes=%u reset=%d\n",
                unsigned(ESP.getFlashChipSize()), unsigned(ESP.getFreeHeap()), unsigned(ESP.getPsramSize()),
                static_cast<int>(esp_reset_reason()));
}
void setup() { Serial.begin(115200); delay(300); report(); Serial.println("Send info + newline"); }
void loop() {
  static char line[32]; static size_t used=0; static bool overflow=false;
  while (Serial.available()) {
    char c=Serial.read();
    if(c=='\r') continue;
    if(c=='\n') {
      line[used]='\0';
      if(overflow) Serial.println("error=line_too_long");
      else if(strcmp(line,"info")==0) report();
      else Serial.println("error=unknown_command");
      used=0; overflow=false;
    } else if(used<sizeof(line)-1) line[used++]=c;
    else overflow=true;
  }
  delay(1);
}
