# 26 SPI通信与SD卡数据记录

[返回总目录](../../README.md) · 前置：先完成17章及正文注明的基础；下一步：总目录第27章。

## 接线与接口选择

前置UART、I²C及文件概念。SPI使用时钟SCK、主机输出MOSI、主机输入MISO、片选CS。本例SCK=18、MISO=19、MOSI=23、CS=13。使用明确支持3.3V逻辑的microSD模块和已格式化为兼容FAT文件系统的卡。不要把裸SD卡接5V，也不要假定带稳压器的模块一定有合适电平转换。

## 时序与吞吐量

SPI的CPOL决定空闲时钟电平，CPHA决定采样边沿，组合成4种模式。外设规格决定模式，主机不能凭“常见模式0”假定所有器件相同。N位数据在时钟f下的理想传输时间为N/f，但SD还需要命令、响应、块传输和内部擦写，不能把4MHz直接当成500kB/s的稳定文件写入承诺。

多个SPI外设可共享SCK/MOSI/MISO，每个有独立CS；未选中的从机应释放MISO。不同器件有不同频率及模式时，使用正确的事务机制，不在另一驱动活动期间任意重配总线。

## 从字节到文件

SD.begin完成卡和文件系统挂载；File对象抽象文件读写。setup只在不存在log.csv时写表头，loop使用FILE_APPEND追加time_ms和heap_bytes，每次flush并close。这里记录的是实际设备运行指标，不伪造温度读数。检查文件打开和写入结果，能区分“初始化成功”和“后续空间耗尽”。

每秒记录约30字节，一天约30×86400≈2.59MB；实际大小取决于数字长度、换行和字段。若每秒100条，日志量和写放大明显增加，可用RAM缓冲批量写入，但断电时会丢失尚未落盘的数据。必须在吞吐、功耗、耐久和可丢失窗口之间取舍。

## 断电一致性

flush可以推进缓存写出，但不能证明任何时刻断电都不会损坏文件系统。生产方案可使用分段文件、校验、序号及双份元数据，并在启动时识别不完整末行。重要控制参数宜用NVS等合适机制，而不与高频日志混写同一文件。移除SD卡前停止写入，不做热插拔演示。

## 实验、排错与练习

运行10秒后正常断电，取卡到Windows/Linux读取CSV，核对表头只有一次且时间递增；再次运行后旧记录仍在。挂载失败先检查线序、CS、供电和文件系统，降低频率可帮助区分信号完整性问题，但不能修复错误供电。需要较大写入电流的卡也可能使板卡电源跌落。

练习：达到1000行后新建下一文件。答案要点是使用持久化或目录扫描获取下一序号，且限定文件名缓冲长度。扩展：每行增加序号和CRC，模拟截断末行后解析器跳过不完整记录，不能静默把半行当成真实测量。

## 完整源码与双平台操作

完整工程：[examples/arduino/sd_logger](../../examples/arduino/sd_logger/README.md)；逐行源码：[main.cpp](../../examples/arduino/sd_logger/src/main.cpp)；依赖与编译配置：[platformio.ini](../../examples/arduino/sd_logger/platformio.ini)。示例固件在ESP32上运行，Windows/Linux负责构建、烧录和监视。主线为经典ESP32-D0WD、4 MB Flash；不假定存在PSRAM或可编程板载LED。

1. 按第04/05章完成工具链安装，在手册根目录打开PlatformIO终端；确认所选环境为esp32dev。
2. 断电按本章接线，检查3.3 V逻辑和共地。只接本例所需部件，避免其他实验复用同一引脚。
3. 如有include/config.example.h，复制为config.local.h并调整自己的配置。联网项目必须修改Wi-Fi凭据；无真实凭据时仍可编译，但不能连接你的网络。
4. 两个平台均执行以下构建与设备枚举命令：

```text
pio run -d examples/arduino/sd_logger
pio device list
```

Windows PowerShell（枚举后输入实际端口）：

```powershell
pio device list
$port = Read-Host '输入枚举出的实际串口'
pio run -d examples/arduino/sd_logger -t upload --upload-port $port
pio device monitor --port $port --baud 115200
```

Linux Bash（将设备路径换成实际枚举结果）：

```bash
pio device list
read -r -p '输入枚举出的实际串口: ' PORT
pio run -d examples/arduino/sd_logger -t upload --upload-port "$PORT"
pio device monitor --port "$PORT" --baud 115200
```

烧录前关闭其他串口监视器。Windows检查CH340驱动和端口占用；Linux检查用户组/udev与文件权限，不用sudo运行整个开发环境，也不把设备改为777权限。按本章预期现象记录实际结果，错误分支也要验证，不能用编译成功代替电路和运行验收。

## 验证状态与来源

代码为本手册原创教学实现，所用框架和第三方库遵循其各自许可证。版本固定在工程配置中；外部教程的API不能跨版本直接混用。
静态阅读与构建、桌面运行、真机运行分别记录，最新实际证据见[validation](../../validation/)。本章没有宣称已经完成OLED、传感器或其他真机测试。

参考：[官方接口或器件资料](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/spi.html)。核对日期：2026-10-29。章节中的推导和例题为教学推导；实际器件参数以相应数据手册为准。
