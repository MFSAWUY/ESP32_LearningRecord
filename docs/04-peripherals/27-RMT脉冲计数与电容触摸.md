# 27 RMT脉冲计数与电容触摸

[返回总目录](../../README.md) · 前置：先完成17章及正文注明的基础；下一步：总目录第28章。

## 目标与接线

本章把发送、计数和触摸作为三种不同外设能力比较。GPIO25连接GPIO27，用RMT输出10个脉冲，由PCNT硬件计数；GPIO4可接一小片导电电极作为经典ESP32触摸输入。GPIO4不接5V或外部强驱动。先只做25→27回环，确认计数，再添加触摸电极，避免同时排查多个变量。

## RMT的时间编码

RMT符号含两个电平和各自持续计数。示例分辨率1MHz，一个计数为1μs；level0=1,duration0=500和level1=0,duration1=500组成1ms周期、50%占空比的脉冲。10个符号总时长10ms。程序每秒发送一次这段脉冲串，所以连续脉冲内频率约1kHz，长期平均脉冲数约10次/秒，两种频率描述不能混淆。

相比用digitalWrite与delayMicroseconds逐边沿生成波形，RMT把边沿时序交给专用外设，降低任务调度干扰。但使用DMA、编码器、内存块或回调时仍有资源限制。本例rmtWrite最多等待100ms，不能在ISR中调用阻塞发送。

## PCNT计数原理与代码

PCNT硬件根据边沿增加、减少或保持计数。示例把上升沿设为INCREASE，下降沿设为HOLD，故10个完整脉冲应得到10而非20。新驱动创建unit和channel，分别设置范围和引脚，然后enable、clear、start。每轮发送后读取并清零；例程的低频脉冲不会在读取和清零之间继续到来。

若测连续转速，读取后清零会在边界漏计风险，应使用持续累计计数及差分，考虑硬件计数器溢出和watch point。转速RPM=60·Δcount/(每转脉冲数·Δt秒)，需要知道编码器每转脉冲定义是单边沿、双边沿还是四倍频。

## 触摸的物理模型

人体靠近电极改变等效电容，从而影响充放电或测量周期，touchRead输出的是与硬件测量方式相关的原始值，不是直接以法拉表示的电容。经典ESP32触摸读数的变化方向应实测确定，不能将另一系列阈值直接复制。先记录未触摸分布，再记录触摸分布，选择有裕量的阈值，并增加滞回和时间去抖。

## 验收与扩展

有回环时每轮pulses应接近精确10；移除跳线后应不再正确累计，浮空输入可能受噪声影响。触摸读数随手指接近变化。若10变20，检查边沿动作；始终0检查接线及初始化返回值。练习：设置20个周期，并用计数结果证明修改；进一步测电机编码器时先引入驱动、供电隔离与输入电平规范，不把本例GPIO直接接电机端子。

## 完整源码与双平台操作

完整工程：[examples/arduino/pulse_touch](../../examples/arduino/pulse_touch/README.md)；逐行源码：[main.cpp](../../examples/arduino/pulse_touch/src/main.cpp)；依赖与编译配置：[platformio.ini](../../examples/arduino/pulse_touch/platformio.ini)。示例固件在ESP32上运行，Windows/Linux负责构建、烧录和监视。主线为经典ESP32-D0WD、4 MB Flash；不假定存在PSRAM或可编程板载LED。

1. 按第04/05章完成工具链安装，在手册根目录打开PlatformIO终端；确认所选环境为esp32dev。
2. 断电按本章接线，检查3.3 V逻辑和共地。只接本例所需部件，避免其他实验复用同一引脚。
3. 如有include/config.example.h，复制为config.local.h并调整自己的配置。联网项目必须修改Wi-Fi凭据；无真实凭据时仍可编译，但不能连接你的网络。
4. 两个平台均执行以下构建与设备枚举命令：

```text
pio run -d examples/arduino/pulse_touch
pio device list
```

Windows PowerShell（枚举后输入实际端口）：

```powershell
pio device list
$port = Read-Host '输入枚举出的实际串口'
pio run -d examples/arduino/pulse_touch -t upload --upload-port $port
pio device monitor --port $port --baud 115200
```

Linux Bash（将设备路径换成实际枚举结果）：

```bash
pio device list
read -r -p '输入枚举出的实际串口: ' PORT
pio run -d examples/arduino/pulse_touch -t upload --upload-port "$PORT"
pio device monitor --port "$PORT" --baud 115200
```

烧录前关闭其他串口监视器。Windows检查CH340驱动和端口占用；Linux检查用户组/udev与文件权限，不用sudo运行整个开发环境，也不把设备改为777权限。按本章预期现象记录实际结果，错误分支也要验证，不能用编译成功代替电路和运行验收。

## 验证状态与来源

代码为本手册原创教学实现，所用框架和第三方库遵循其各自许可证。版本固定在工程配置中；外部教程的API不能跨版本直接混用。
静态阅读与构建、桌面运行、真机运行分别记录，最新实际证据见[validation](../../validation/)。本章没有宣称已经完成OLED、传感器或其他真机测试。

参考：[官方接口或器件资料](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/rmt.html)。核对日期：2026-10-29。章节中的推导和例题为教学推导；实际器件参数以相应数据手册为准。
