#pragma once
#include "settings.h"
#include <Wire.h>
#include <U8g2lib.h>
#if OLED_CONTROLLER == 1106
#if OLED_HEIGHT != 64
#error "This SH1106 profile is 128x64; use a matching U8g2 constructor for other modules."
#endif
U8G2_SH1106_128X64_NONAME_F_HW_I2C screen(U8G2_R0, U8X8_PIN_NONE);
#elif OLED_HEIGHT == 32
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C screen(U8G2_R0, U8X8_PIN_NONE);
#else
U8G2_SSD1306_128X64_NONAME_F_HW_I2C screen(U8G2_R0, U8X8_PIN_NONE);
#endif
bool displayReady = false;
void startDisplay() {
    Wire.begin(21, 22);
    Wire.setTimeOut(50);
    Wire.beginTransmission(OLED_ADDRESS);
    displayReady = Wire.endTransmission() == 0;
    if (!displayReady) { Serial.println("OLED absent: check address and wiring"); return; }
    screen.setI2CAddress(OLED_ADDRESS << 1); // U8g2 uses the shifted 8-bit address
    screen.begin();
    screen.setFont(u8g2_font_6x12_tf);
}
