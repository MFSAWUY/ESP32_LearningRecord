# 新 I²C 驱动读取 SHT30

经典 ESP32 / 4 MB Flash / ESP-IDF 6.1，无PSRAM依赖。对应手册第28、42章。

## 接线、参数与代码讲解

| ESP32 | SHT30模块 |
|---|---|
| 3V3 / GND | VCC / GND，确认模块支持3.3V |
| GPIO21 / GPIO22 | SDA / SCL |

默认7位地址0x44、100kHz；ADDR拉高的模块可能为0x45，先扫描。总线需要合适的外部上拉；模块已有上拉时先确认，不盲目并联许多电阻。代码启用的内部弱上拉不能替代所有布线条件下的外部上拉。

代码先创建总线，再注册设备；发送0x2400触发高重复性单次测量，等待20ms，再读取6字节。温度与湿度各有两字节数据和一字节CRC。CRC初值0xFF、多项式0x31，校验失败不发布读数。换算为T=−45+175×raw/65535，RH=100×raw/65535。

预期每2秒记录`temperature=... C humidity=... %RH`。拔除传感器会打印错误，重新连接后后续周期可恢复。每次I²C事务有100ms超时；正常的缺失设备不是断言复位条件。

练习：把地址改成错误值，验证失败可观察；然后恢复。模块化时让读取函数返回状态和数据，避免用0℃表示失败。

## 从打开工程到运行

1. 在VS Code或终端打开本工程目录，先按第04/05章激活ESP-IDF 6.1环境。
2. 按[Windows PowerShell与Linux Bash完整命令](../README.md)首次设置esp32目标、构建，再枚举串口并输入实际端口。构建本身不要求连接开发板。
3. 按本页接线，烧录后以115200波特率观察日志；关闭其他占用同一串口的软件。
4. 核对上述预期现象与错误路径，并填写真机验收表。预期日志是教学说明，不是真机运行证据。

源码入口为[main/main.c](main/main.c)，组件依赖见[main/CMakeLists.txt](main/CMakeLists.txt)，默认目标和Flash参数见[sdkconfig.defaults](sdkconfig.defaults)。根CMake启用MINIMAL_BUILD，只构建main及声明的依赖；依赖缺失应修正组件清单，不靠无关组件偶然传递头文件。

## 来源与验证

教学代码为本手册原创，采用[MIT](../../../LICENSE)；ESP-IDF及其组件保持各自许可证。API参考[ESP-IDF v6.1官方文档](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/)。实际编译状态见[验证报告](../../../validation/REPORT.md)，物理实验填[硬件验收表](../../../validation/HARDWARE_CHECKLIST.md)。
