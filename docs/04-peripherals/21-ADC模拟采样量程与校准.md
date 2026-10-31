# 21 ADC模拟采样量程与校准

[返回总目录](../../README.md) · 前置：先完成17章及正文注明的基础；下一步：总目录第22章。

## 实验目标和接线

前置第06、10、18章。10kΩ电位器两端接3V3与GND，中间滑片接GPIO34。GPIO34属于ADC1，输入专用；本例不依赖内部上拉。把滑片从低端缓慢转向高端，观察raw及mv整体增长。GPIO允许的电气范围与ADC可准确测量的范围是两个概念，不应通过提高输入电压来强行获得满码。

## 从连续电压到离散码

理想N位ADC有2^N个码，若满量程跨度为V_FS，理想量化步长q=V_FS/2^N。理想四舍五入量化的误差通常在±q/2内；这不包含参考误差、非线性、噪声和输入电路误差。使用端点映射V≈code·V_FS/(2^N−1)是另一种近似换算约定，应与量化步长区分。

例：假设理想12位ADC跨度3.0V，q≈0.732mV。这个例子不是宣称ESP32在全部衰减档都以3.0V为满量程。真实ESP32需要依据芯片文档、衰减配置和校准路径处理，因此代码同时输出analogRead原始码与analogReadMilliVolts转换值，避免直接把全部原始码乘3.3/4095。

## 代码阅读

analogReadResolution(12)指定返回分辨率；analogSetPinAttenuation(34,ADC_11db)选择衰减档，增大可测范围同时可能改变误差特征。16次采样分别累加，最后除16.0避免整数截断。原始码和毫伏是两个独立调用得到的样本，快速变化信号不能把它们解释为完全同一采样时刻的转换对。

在无相关零均值随机噪声的假设下，M次平均可将标准差约降到σ/√M；M=16时约变成原来的1/4。平均不能消除直流偏差，也不能恢复饱和丢失的信息。慢速示例的delay仅控制观察频率，连续高速采样应使用专门驱动和DMA，不由任意loop调用保证固定采样率。

## 测量方法与误差来源

用万用表同时测滑片对GND电压，在多个点记录原始码、毫伏值和表读数。不要只测0V及顶端，因为中间区域的非线性可能被端点测试遗漏。电位器端接3V3方便学习，但高端可能超出选定ADC档的准确范围；出现平台是诊断信号。参考电源纹波、源阻抗、导线耦合和采样电容充电都会影响结果。

经典ESP32的ADC2会与Wi-Fi争用，因此联网模拟输入优先选择ADC1。浮空输入有随机读数并不等于芯片损坏。接入5V模拟传感器必须先设计分压或缓冲，验证最大输出和故障状态，不能直接接入GPIO。

## 练习与答案要点

取两个可靠参考点(V1,C1)、(V2,C2)，求线性校准V=aC+b，其中a=(V2−V1)/(C2−C1)，b=V1−aC1。用第三个未参与拟合的点验证残差，避免用拟合点证明准确性。若残差明显随输入变化，应讨论多点校准或更适合的外置ADC，而不是增加显示小数位。

## 完整源码与双平台操作

完整工程：[examples/arduino/adc](../../examples/arduino/adc/README.md)；逐行源码：[main.cpp](../../examples/arduino/adc/src/main.cpp)；依赖与编译配置：[platformio.ini](../../examples/arduino/adc/platformio.ini)。示例固件在ESP32上运行，Windows/Linux负责构建、烧录和监视。主线为经典ESP32-D0WD、4 MB Flash；不假定存在PSRAM或可编程板载LED。

1. 按第04/05章完成工具链安装，在手册根目录打开PlatformIO终端；确认所选环境为esp32dev。
2. 断电按本章接线，检查3.3 V逻辑和共地。只接本例所需部件，避免其他实验复用同一引脚。
3. 如有include/config.example.h，复制为config.local.h并调整自己的配置。联网项目必须修改Wi-Fi凭据；无真实凭据时仍可编译，但不能连接你的网络。
4. 两个平台均执行以下构建与设备枚举命令：

```text
pio run -d examples/arduino/adc
pio device list
```

Windows PowerShell（枚举后输入实际端口）：

```powershell
pio device list
$port = Read-Host '输入枚举出的实际串口'
pio run -d examples/arduino/adc -t upload --upload-port $port
pio device monitor --port $port --baud 115200
```

Linux Bash（将设备路径换成实际枚举结果）：

```bash
pio device list
read -r -p '输入枚举出的实际串口: ' PORT
pio run -d examples/arduino/adc -t upload --upload-port "$PORT"
pio device monitor --port "$PORT" --baud 115200
```

烧录前关闭其他串口监视器。Windows检查CH340驱动和端口占用；Linux检查用户组/udev与文件权限，不用sudo运行整个开发环境，也不把设备改为777权限。按本章预期现象记录实际结果，错误分支也要验证，不能用编译成功代替电路和运行验收。

## 验证状态与来源

代码为本手册原创教学实现，所用框架和第三方库遵循其各自许可证。版本固定在工程配置中；外部教程的API不能跨版本直接混用。
静态阅读与构建、桌面运行、真机运行分别记录，最新实际证据见[validation](../../validation/)。本章没有宣称已经完成OLED、传感器或其他真机测试。

参考：[官方接口或器件资料](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html)。核对日期：2026-10-29。章节中的推导和例题为教学推导；实际器件参数以相应数据手册为准。
