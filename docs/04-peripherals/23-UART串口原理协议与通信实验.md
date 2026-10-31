# 23 UART串口原理协议与通信实验

[返回总目录](../../README.md) · 前置：先完成17章及正文注明的基础；下一步：总目录第24章。

## 接线和目标

前置18章与位运算。USB上的CH340连接UART下载与日志通道，本例另外使用UART2：TX为GPIO17，RX为GPIO16。用一根杜邦线将17连接16即可回环。若连接外部设备，则TX接对方RX、RX接对方TX，并共地。3.3V TTL UART不是RS232电压接口，也不是USB，不可仅因都称“串口”就直接连接。

## 帧结构与吞吐量

8N1表示8数据位、无校验、1停止位，另有1起始位，每字节至少占10位。9600bit/s在连续满载时理论最多约960字节/秒，115200bit/s约11520字节/秒；帧间隔、应用处理和协议字段还会进一步降低有效吞吐。发送20字节一行、每秒1000行需约20kB/s，115200不能承载。

UART没有共享时钟，接收端通过起始位确定采样时间。双方波特率差会积累到帧后部，过大误差导致停止位错误。数据格式同样重要：一方8N1另一方7E1不会因为波特率相同就正确。

## 字节流到应用协议

UART给出的是字节流，单次read不保证获得完整命令。本例发送PING序号和换行，接收端逐字节放入64字节缓冲，遇到\n才交付一行，忽略\r兼容两种换行。used<sizeof(line)−1为终止符留空间；超长帧标记overflow并丢弃到下一分隔符。

```cpp
link.begin(9600, SERIAL_8N1, 16, 17);
link.printf("PING %u\n", seq++);
```

这两行分别定义电气接口参数和应用报文。对真实设备，应进一步定义命令号、长度、序号、校验和超时，不能把“能收到字符”当成协议完整。二进制协议可用固定头+长度+负载+CRC；CRC检测传输错误，不提供身份认证。

## 实验步骤和故障定位

先不短接，确认USB日志正常但没有RX行；短接17与16后应每秒收到对应PING。移除跳线应停止回显，但设备继续运行。交换外部设备TX/RX可验证线序错误表现。不要把调试UART0的GPIO1/3和本例UART2混用，否则可能影响烧录和日志。

示例对普通WROOM且无PSRAM的经典板卡适用。其他模组可能使用16/17连接内部存储，移植必须查模组原理图。线很长、强干扰或不同设备地电位差较大时，应考虑RS485收发器等合适物理层，而非单纯降低波特率。

## 练习与参考答案

增加ACK序号及500ms超时，收到错误序号不确认上一条命令。答案应维护待确认序号和发送时刻，不能每收到任意字节就判成功。再增加128字节恶意输入，验收后续正常PING仍能恢复，证明缓冲边界和重新同步有效。

## 完整源码与双平台操作

完整工程：[examples/arduino/uart](../../examples/arduino/uart/README.md)；逐行源码：[main.cpp](../../examples/arduino/uart/src/main.cpp)；依赖与编译配置：[platformio.ini](../../examples/arduino/uart/platformio.ini)。示例固件在ESP32上运行，Windows/Linux负责构建、烧录和监视。主线为经典ESP32-D0WD、4 MB Flash；不假定存在PSRAM或可编程板载LED。

1. 按第04/05章完成工具链安装，在手册根目录打开PlatformIO终端；确认所选环境为esp32dev。
2. 断电按本章接线，检查3.3 V逻辑和共地。只接本例所需部件，避免其他实验复用同一引脚。
3. 如有include/config.example.h，复制为config.local.h并调整自己的配置。联网项目必须修改Wi-Fi凭据；无真实凭据时仍可编译，但不能连接你的网络。
4. 两个平台均执行以下构建与设备枚举命令：

```text
pio run -d examples/arduino/uart
pio device list
```

Windows PowerShell（枚举后输入实际端口）：

```powershell
pio device list
$port = Read-Host '输入枚举出的实际串口'
pio run -d examples/arduino/uart -t upload --upload-port $port
pio device monitor --port $port --baud 115200
```

Linux Bash（将设备路径换成实际枚举结果）：

```bash
pio device list
read -r -p '输入枚举出的实际串口: ' PORT
pio run -d examples/arduino/uart -t upload --upload-port "$PORT"
pio device monitor --port "$PORT" --baud 115200
```

烧录前关闭其他串口监视器。Windows检查CH340驱动和端口占用；Linux检查用户组/udev与文件权限，不用sudo运行整个开发环境，也不把设备改为777权限。按本章预期现象记录实际结果，错误分支也要验证，不能用编译成功代替电路和运行验收。

## 验证状态与来源

代码为本手册原创教学实现，所用框架和第三方库遵循其各自许可证。版本固定在工程配置中；外部教程的API不能跨版本直接混用。
静态阅读与构建、桌面运行、真机运行分别记录，最新实际证据见[validation](../../validation/)。本章没有宣称已经完成OLED、传感器或其他真机测试。

参考：[官方接口或器件资料](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/serial.html)。核对日期：2026-10-29。章节中的推导和例题为教学推导；实际器件参数以相应数据手册为准。
