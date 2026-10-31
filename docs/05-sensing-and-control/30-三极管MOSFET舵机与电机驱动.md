# 30 三极管MOSFET舵机与电机驱动

[返回总目录](../../README.md) · 前置：先完成17章及正文注明的基础；下一步：总目录第31章。

## 实验范围与接线

本章先用舵机控制信号学习执行器，代码默认输出关闭，只在串口收到命令后输出。舵机信号接GPIO25，舵机电源由符合规格的独立电源提供，GND与ESP32共地。确认舵机能识别3.3V输入；不要从GPIO供电，也不要以USB连接器能提供5V为由直接承诺足够电流。电机堵转电流可能远高于空载电流。

## 为什么需要驱动级

GPIO适合提供控制信号，负载的电压和电流由驱动级承担。以N沟道MOSFET低侧开关为例，负载正端接电源，负端接漏极，源极接地，栅极由GPIO经合适电阻控制，并用下拉保证复位时关闭。选择MOSFET要看3.3V栅压下的导通电阻，而不是只看阈值电压V_GS(th)。阈值表示刚开始形成小电流，不代表已经充分导通。

导通损耗近似P=I²R_DS(on)。例如1A、0.1Ω对应0.1W；若电流变3A，损耗变0.9W。还要考虑开关损耗、温升和散热。双极型三极管需要基极电流，应根据饱和设计而非理想放大倍数直接计算；高电流负载优先使用适当驱动模块。

电机线圈电流不能瞬间变化，关断时L·di/dt会产生高压，因此直流感性负载常需续流路径。单向低侧开关的二极管通常反向并在负载两端；双向H桥的回流路径由驱动拓扑决定，不能机械地把同一连接照搬到所有电机系统。

## 舵机脉宽推导和代码

常见舵机使用周期约20ms的控制脉冲，示例只提供1000、1500、2000μs三档，实际允许范围和角度关系以舵机说明为准。不能将所有舵机的1000～2000μs都等同0～180°。16位PWM计数下，duty≈pulse_us×65536/20000，1500μs对应约4915。

串口发送0/1/2选择脉宽，x停止脉冲。三秒没有新命令自动关闭输出，这演示失联策略；关闭信号不一定让舵机机械回到安全位置，也不等同切断电源。真正系统应根据机械风险设计归位、制动或断电方案。

## 验收、故障与练习

先不接舵机，用逻辑分析仪确认频率和高电平宽度，再接舵机做无负载小范围试验。转动时ESP32重启通常需要检查供电跌落、共地和干扰。练习：让目标脉宽按每20ms最多增加10μs变化；答案要点是限速状态机，而不是从最小值瞬间跳到最大值。电机闭环控制另需编码器和驱动器，本章的舵机PWM不能直接代替直流电机功率PWM。

## 完整源码与双平台操作

完整工程：[examples/arduino/actuator](../../examples/arduino/actuator/README.md)；逐行源码：[main.cpp](../../examples/arduino/actuator/src/main.cpp)；依赖与编译配置：[platformio.ini](../../examples/arduino/actuator/platformio.ini)。示例固件在ESP32上运行，Windows/Linux负责构建、烧录和监视。主线为经典ESP32-D0WD、4 MB Flash；不假定存在PSRAM或可编程板载LED。

1. 按第04/05章完成工具链安装，在手册根目录打开PlatformIO终端；确认所选环境为esp32dev。
2. 断电按本章接线，检查3.3 V逻辑和共地。只接本例所需部件，避免其他实验复用同一引脚。
3. 如有include/config.example.h，复制为config.local.h并调整自己的配置。联网项目必须修改Wi-Fi凭据；无真实凭据时仍可编译，但不能连接你的网络。
4. 两个平台均执行以下构建与设备枚举命令：

```text
pio run -d examples/arduino/actuator
pio device list
```

Windows PowerShell（枚举后输入实际端口）：

```powershell
pio device list
$port = Read-Host '输入枚举出的实际串口'
pio run -d examples/arduino/actuator -t upload --upload-port $port
pio device monitor --port $port --baud 115200
```

Linux Bash（将设备路径换成实际枚举结果）：

```bash
pio device list
read -r -p '输入枚举出的实际串口: ' PORT
pio run -d examples/arduino/actuator -t upload --upload-port "$PORT"
pio device monitor --port "$PORT" --baud 115200
```

烧录前关闭其他串口监视器。Windows检查CH340驱动和端口占用；Linux检查用户组/udev与文件权限，不用sudo运行整个开发环境，也不把设备改为777权限。按本章预期现象记录实际结果，错误分支也要验证，不能用编译成功代替电路和运行验收。

## 验证状态与来源

代码为本手册原创教学实现，所用框架和第三方库遵循其各自许可证。版本固定在工程配置中；外部教程的API不能跨版本直接混用。
静态阅读与构建、桌面运行、真机运行分别记录，最新实际证据见[validation](../../validation/)。本章没有宣称已经完成OLED、传感器或其他真机测试。

参考：[官方接口或器件资料](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ledc.html)。核对日期：2026-10-29。章节中的推导和例题为教学推导；实际器件参数以相应数据手册为准。
