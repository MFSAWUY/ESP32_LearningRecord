# 19 GPIO输入输出LED与按键去抖

[返回总目录](../../README.md) · 前置：先完成17章及正文注明的基础；下一步：总目录第20章。

## 目标、前置与接线

前置第06、07、18章。目标是按一下按键只计一次并切换LED，松手不切换。外接LED：GPIO25→330Ω→LED阳极，阴极→GND；按键连接GPIO27与GND。四脚按键同侧引脚可能内部短接，应先用万用表确认，不能只凭外观。INPUT_PULLUP使未按下时为高电平，按下后为低电平，这称为低有效。

## 电路推导与边界

假设GPIO输出近似3.3V，红色LED正向压降约2.0V，忽略导线电阻，串联回路有3.3=I·R+2.0。若R=330Ω，I≈3.94mA。真实压降随器件、温度和电流变化，因此计算是选型起点；不同颜色LED不应共用同一个压降假设。电阻耗散P=I²R≈5.1mW，仍需满足电阻规格和GPIO驱动限制。

机械触点首次接触后会在短时间内反复开闭，直接在每次下降沿累加会得到多次事件。软件去抖采用“候选值保持稳定30ms才接受”，它过滤的是时间上短暂的状态变化，而不是模拟低通电路。30ms是教学起点，应按实测按钮和响应要求修改。

## 状态机与代码解释

previous保存上一轮原始采样值，stable保存已经接受的逻辑状态，changed记录最近一次原始变化的时间。检测到raw变化就重新计时；只有now-changed≥30且stable≠raw，才更新stable。接着只在LOW状态计数，故松手不会再次触发。

```cpp
if (raw != previous) { previous = raw; changed = now; }
if (now - changed >= 30 && stable != raw) {
    stable = raw;
    if (stable == LOW) { /* 接受一次按下事件 */ }
}
```

now和changed必须使用同样的无符号类型。无符号减法按模2^32运算，只要检查间隔远小于完整回绕周期，即使millis从最大值回到0也能计算短时间间隔。用now>changed+30可能在加法回绕时失败。外设代码处理事件后立刻返回，方便以后同时刷新OLED或处理网络。

## 操作与观察

先用GPIO输出固定HIGH确认电路，再运行完整程序。连续按10次应记录10次；长按2秒应只增加一次；松开不增加。快速但完整的重复按压可以计数，短于去抖窗口的动作可能被有意过滤。实验记录应同时包含次数和按压方式。

一直读取LOW可能是按键接线短路；随机变化可能是没有上拉、共地不良或线过长；LED反向不会亮。GPIO34～39只能输入且无内部上拉下拉，不应直接用同样代码替代27。启动配置引脚也不作为默认按钮选择。

## 练习与参考答案

加入长按1秒清零计数。记录已去抖的按下时刻，再用longHandled标志保证一次长按只触发一次；松开时清除标志。不要在按下分支内delay(1000)，否则期间其他任务和松手事件无法及时处理。

## 完整源码与双平台操作

完整工程：[examples/arduino/gpio_button](../../examples/arduino/gpio_button/README.md)；逐行源码：[main.cpp](../../examples/arduino/gpio_button/src/main.cpp)；依赖与编译配置：[platformio.ini](../../examples/arduino/gpio_button/platformio.ini)。示例固件在ESP32上运行，Windows/Linux负责构建、烧录和监视。主线为经典ESP32-D0WD、4 MB Flash；不假定存在PSRAM或可编程板载LED。

1. 按第04/05章完成工具链安装，在手册根目录打开PlatformIO终端；确认所选环境为esp32dev。
2. 断电按本章接线，检查3.3 V逻辑和共地。只接本例所需部件，避免其他实验复用同一引脚。
3. 如有include/config.example.h，复制为config.local.h并调整自己的配置。联网项目必须修改Wi-Fi凭据；无真实凭据时仍可编译，但不能连接你的网络。
4. 两个平台均执行以下构建与设备枚举命令：

```text
pio run -d examples/arduino/gpio_button
pio device list
```

Windows PowerShell（枚举后输入实际端口）：

```powershell
pio device list
$port = Read-Host '输入枚举出的实际串口'
pio run -d examples/arduino/gpio_button -t upload --upload-port $port
pio device monitor --port $port --baud 115200
```

Linux Bash（将设备路径换成实际枚举结果）：

```bash
pio device list
read -r -p '输入枚举出的实际串口: ' PORT
pio run -d examples/arduino/gpio_button -t upload --upload-port "$PORT"
pio device monitor --port "$PORT" --baud 115200
```

烧录前关闭其他串口监视器。Windows检查CH340驱动和端口占用；Linux检查用户组/udev与文件权限，不用sudo运行整个开发环境，也不把设备改为777权限。按本章预期现象记录实际结果，错误分支也要验证，不能用编译成功代替电路和运行验收。

## 验证状态与来源

代码为本手册原创教学实现，所用框架和第三方库遵循其各自许可证。版本固定在工程配置中；外部教程的API不能跨版本直接混用。
静态阅读与构建、桌面运行、真机运行分别记录，最新实际证据见[validation](../../validation/)。本章没有宣称已经完成OLED、传感器或其他真机测试。

参考：[官方接口或器件资料](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/gpio.html)。核对日期：2026-10-29。章节中的推导和例题为教学推导；实际器件参数以相应数据手册为准。
