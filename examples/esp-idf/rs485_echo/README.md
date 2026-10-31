# RS485 半双工字节回显

经典 ESP32 / 4 MB Flash / ESP-IDF 6.1，无PSRAM依赖。对应手册第65章。

## 接线、参数与代码讲解

需要3.3V逻辑RS485收发器和另一端USB-RS485适配器；GPIO不能直接接A/B。两端波特率19200、8N1。

| ESP32 | 收发器 |
|---|---|
| GPIO17 | DI（发送数据） |
| GPIO16 | RO（接收数据） |
| GPIO27 | DE与低有效/RE连接点 |
| GND | 信号参考地 |

A/B按双方标识和资料连接；终端电阻位于总线两个物理末端，偏置按总线设计，不给每个节点都加重复偏置。不要使用输出5V逻辑的收发器直接驱动ESP32。

UART2以RS485半双工模式控制RTS方向。读取最多128字节，等待发送方释放总线后回显；发送完成最长等待100ms。不把一次read当成完整Modbus帧：本例只验证物理层和字节往返，没有地址、功能码、CRC和异常响应。

电脑端用支持原始串口的终端发送短ASCII消息，应收到相同字节，ESP32日志报告接收长度。两端都回显会形成循环，所以电脑端关闭自动回显转发。超时查方向、极性、参数、参考地与端接。

练习：引入明确的换行分帧与长度上限，再在第65章原则下学习Modbus RTU时序和CRC。

## 从打开工程到运行

1. 在VS Code或终端打开本工程目录，先按第04/05章激活ESP-IDF 6.1环境。
2. 按[Windows PowerShell与Linux Bash完整命令](../README.md)首次设置esp32目标、构建，再枚举串口并输入实际端口。构建本身不要求连接开发板。
3. 按本页接线，烧录后以115200波特率观察日志；关闭其他占用同一串口的软件。
4. 核对上述预期现象与错误路径，并填写真机验收表。预期日志是教学说明，不是真机运行证据。

源码入口为[main/main.c](main/main.c)，组件依赖见[main/CMakeLists.txt](main/CMakeLists.txt)，默认目标和Flash参数见[sdkconfig.defaults](sdkconfig.defaults)。根CMake启用MINIMAL_BUILD，只构建main及声明的依赖；依赖缺失应修正组件清单，不靠无关组件偶然传递头文件。

## 来源与验证

教学代码为本手册原创，采用[MIT](../../../LICENSE)；ESP-IDF及其组件保持各自许可证。API参考[ESP-IDF v6.1官方文档](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/)。实际编译状态见[验证报告](../../../validation/REPORT.md)，物理实验填[硬件验收表](../../../validation/HARDWARE_CHECKLIST.md)。
