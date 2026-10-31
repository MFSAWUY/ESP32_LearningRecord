#pragma once
#include <Wire.h>
#include <math.h>
struct Reading { bool valid; float temperature; float humidity; };
uint8_t shtCrc(const uint8_t* data, size_t count) {
    uint8_t crc = 0xff;
    while(count--) { crc ^= *data++; for(int bit=0; bit<8; ++bit) crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1; }
    return crc;
}
Reading readSht30() {
    Reading reading{false, NAN, NAN};
    Wire.beginTransmission(0x44);
    Wire.write(0x24); Wire.write(0x00); // high repeatability, no clock stretching
    if (Wire.endTransmission() != 0) return reading;
    delay(20); // bounded sensor conversion wait; not inside an ISR
    if (Wire.requestFrom(uint8_t(0x44), uint8_t(6)) != 6) return reading;
    uint8_t data[6]; for(auto& byte : data) byte = Wire.read();
    if(shtCrc(data,2)!=data[2] || shtCrc(data+3,2)!=data[5]) return reading;
    uint16_t rawT = (uint16_t(data[0])<<8)|data[1];
    uint16_t rawH = (uint16_t(data[3])<<8)|data[4];
    reading.temperature = -45.0f + 175.0f*rawT/65535.0f;
    reading.humidity = 100.0f*rawH/65535.0f;
    reading.valid = true;
    return reading;
}
