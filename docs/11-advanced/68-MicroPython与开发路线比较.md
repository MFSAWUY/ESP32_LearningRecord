# 68 MicroPython与开发路线比较

[返回总目录](../../README.md)

串口先枚举再选择；本章端口变量的初始化见[Windows/Linux公共命令](../../examples/esp-idf/README.md)。

## 为什么作为选学

前置：第12、24、54章。你已经会Python，可以用MicroPython快速探索GPIO和I²C，再与Arduino/ESP-IDF对照。MicroPython运行在ESP32的解释器固件中，电脑CPython运行上位机脚本；两者并非同一套完整标准库环境。随附独立程序：[i2c_led.py](../../examples/micropython/i2c_led.py)。

| 路线 | 优点 | 需要关注 |
|---|---|---|
| Arduino C++ | 外设库多、快速搭建、编译成机器码 | 框架抽象、库兼容性和隐含任务 |
| ESP-IDF C/C++ | 官方组件、配置和系统控制完整 | 构建、资源生命周期与并发复杂度 |
| MicroPython | REPL交互、修改快、适合原型 | 解释器开销、GC暂停、内存及模块覆盖 |

Python代码简短不等于时序更确定。需要严格脉宽时应使用硬件PWM/RMT等外设，不依赖解释器循环翻转引脚；中断中避免动态分配和耗时打印。高层状态机可用Python实现，实时部分交给硬件或经过验证的原生模块。

## 安装边界与运行

先从[官方ESP32固件页面](https://micropython.org/download/ESP32_GENERIC/)选择匹配经典ESP32的稳定固件，记录版本和SHA256，并遵循该发布页面给出的烧录地址。更换解释器会替换当前Arduino/IDF固件，涉及擦除时会丢失对应Flash数据；先备份配置并选专用学习时段。这里不自动执行擦除或烧录。

固件正常后，在独立电脑虚拟环境安装官方mpremote工具并记录安装版本。Windows使用虚拟环境python执行`-m mpremote connect $serialPort run examples/micropython/i2c_led.py`；Linux对应`-m mpremote connect "$serial_port" run examples/micropython/i2c_led.py`。实际端口须枚举，且要关闭其他串口监视器。本脚本通过run临时执行，不默认覆盖设备main.py。

## 程序讲解

machine.Pin配置GPIO25为输出；LED接串联330Ω电阻到地。machine.I2C选择SDA21/SCL22和100kHz，scan返回7位设备地址。代码每秒翻转LED并打印扫描结果，使用ticks_diff比较时刻，处理计时回绕，而不是直接假设ticks_ms永远递增。没有外接LED也可观察扫描结果；OLED ACK只确认设备回应，不确定其控制器型号。

finally将LED置低，便于Ctrl+C停止后的状态可预测。OSError捕捉总线失败并记录，但不会伪造设备地址。无限循环中保留sleep_ms，让系统有时间调度；需要更复杂并发时学习uasyncio，同时认识到合作式协程不自动提供硬实时保证。

## 内存与实践

MicroPython对象有额外管理开销，1000个Python整数的占用通常大于1000个C int32。避免高频循环反复拼接长字符串和增长列表，提前分配缓冲，必要时测量gc.mem_free。手工gc.collect可以把回收安排到合适时间，但不能凭这一点宣称无抖动。

练习：将I²C扫描周期改为5秒，LED仍每秒翻转。参考：维护两个独立截止时间，不能用sleep_ms(5000)阻塞整个循环。进一步将同样需求分别用Arduino millis和FreeRTOS任务实现，比较状态、资源和时序。来源：[MicroPython ESP32快速参考](https://docs.micropython.org/en/latest/esp32/quickref.html)、[mpremote文档](https://docs.micropython.org/en/latest/reference/mpremote.html)。下一步：第69章排障与复习。

> 编译和真机状态见[验证报告](../../validation/REPORT.md)。额外硬件未连接时，不将理论或软件仿真当作实测。
