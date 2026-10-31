"""Run on ESP32 MicroPython, not desktop CPython."""
from machine import Pin, I2C
from time import ticks_ms, ticks_diff, sleep_ms

led = Pin(25, Pin.OUT, value=0)
bus = I2C(0, sda=Pin(21), scl=Pin(22), freq=100000)
last = ticks_ms()
try:
    while True:
        now = ticks_ms()
        if ticks_diff(now, last) >= 1000:
            last = now
            led.value(1 - led.value())
            try:
                print("I2C addresses:", [hex(address) for address in bus.scan()])
            except OSError as error:
                print("I2C error:", error)
        sleep_ms(10)
finally:
    led.value(0)
