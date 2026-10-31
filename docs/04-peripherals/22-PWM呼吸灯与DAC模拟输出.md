# 22 PWM呼吸灯与DAC模拟输出

[返回总目录](../../README.md) · 前置：先完成17章及正文注明的基础；下一步：总目录第23章。

## 接线、目标和功能边界

前置ADC与LED章节。GPIO25经330Ω驱动LED，GPIO26作为DAC输出，用高阻万用表或示波器观察；不要将DAC直接接电机、扬声器等低阻负载。本章只针对经典ESP32拥有的DAC外设，其他系列不能直接照搬。程序在GPIO25输出PWM，在GPIO26输出8位DAC，便于比较两种输出。

## PWM推导

周期T内高电平持续t_H，占空比D=t_H/T，频率f=1/T。理想0～VDD矩形波在一个周期的平均值为(1/T)∫v(t)dt=D·VDD。这是数学平均值；负载是否感受到近似直流，还取决于低通滤波、负载惯性和纹波。单独GPIO上仍然是快速高低跳变。

若计数器有N位，每周期有2^N个时隙，近似f_PWM=f_clk/(divider·2^N)。提高分辨率会压缩可实现的频率范围；不是频率和位数都越大越好。示例选择5kHz、8位，ledcWrite写入0～255。接口对满占空比的特殊处理应以驱动为准，计数分辨率与显示百分比需要统一约定。

假设3.3V、D=0.25，理想平均值为0.825V，但万用表采样方法可能使读数偏差。LED的视觉亮度与电功率关系也非简单线性；线性递增占空比通常不产生完全均匀的主观亮度。可用查表实现感知亮度修正，但不能把查表当成电气校准。

## 代码解释与实验

ledcAttach(PWM_PIN,5000,8)把频率和分辨率绑定到引脚，返回false时停止输出逻辑。Arduino3.x的ledcWrite使用引脚，本例不使用旧版ledcSetup+ledcAttachPin。value从0增长到255后改变step方向，每10ms更新；DAC同时输出相同码值。DAC理想输出近似随码值线性变化，实际受参考电源、输出阻抗和偏差限制。

观察LED渐亮渐暗；在GPIO25看见近似固定周期、改变高电平宽度的波形；GPIO26则是阶梯状缓慢变化的电压。对PWM加RC滤波时，时间常数τ远大于PWM周期可降低纹波，但会降低响应速度，正好联系第07章。

## 排错与练习

LED不亮先用GPIO固定输出排除极性和电阻问题。频率配置失败检查时钟与分辨率组合；多个PWM输出可能共享定时器，修改频率会影响同组通道。练习：维持5kHz将分辨率改为10位，同时把占空比范围和步长改为0～1023；DAC仍只接受0～255，需独立缩放。答案要点是区分两个输出的数值域，而不是仅修改ledcAttach的第三个参数。

## 完整源码与双平台操作

完整工程：[examples/arduino/pwm_dac](../../examples/arduino/pwm_dac/README.md)；逐行源码：[main.cpp](../../examples/arduino/pwm_dac/src/main.cpp)；依赖与编译配置：[platformio.ini](../../examples/arduino/pwm_dac/platformio.ini)。示例固件在ESP32上运行，Windows/Linux负责构建、烧录和监视。主线为经典ESP32-D0WD、4 MB Flash；不假定存在PSRAM或可编程板载LED。

1. 按第04/05章完成工具链安装，在手册根目录打开PlatformIO终端；确认所选环境为esp32dev。
2. 断电按本章接线，检查3.3 V逻辑和共地。只接本例所需部件，避免其他实验复用同一引脚。
3. 如有include/config.example.h，复制为config.local.h并调整自己的配置。联网项目必须修改Wi-Fi凭据；无真实凭据时仍可编译，但不能连接你的网络。
4. 两个平台均执行以下构建与设备枚举命令：

```text
pio run -d examples/arduino/pwm_dac
pio device list
```

Windows PowerShell（枚举后输入实际端口）：

```powershell
pio device list
$port = Read-Host '输入枚举出的实际串口'
pio run -d examples/arduino/pwm_dac -t upload --upload-port $port
pio device monitor --port $port --baud 115200
```

Linux Bash（将设备路径换成实际枚举结果）：

```bash
pio device list
read -r -p '输入枚举出的实际串口: ' PORT
pio run -d examples/arduino/pwm_dac -t upload --upload-port "$PORT"
pio device monitor --port "$PORT" --baud 115200
```

烧录前关闭其他串口监视器。Windows检查CH340驱动和端口占用；Linux检查用户组/udev与文件权限，不用sudo运行整个开发环境，也不把设备改为777权限。按本章预期现象记录实际结果，错误分支也要验证，不能用编译成功代替电路和运行验收。

## 验证状态与来源

代码为本手册原创教学实现，所用框架和第三方库遵循其各自许可证。版本固定在工程配置中；外部教程的API不能跨版本直接混用。
静态阅读与构建、桌面运行、真机运行分别记录，最新实际证据见[validation](../../validation/)。本章没有宣称已经完成OLED、传感器或其他真机测试。

参考：[官方接口或器件资料](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ledc.html)。核对日期：2026-10-29。章节中的推导和例题为教学推导；实际器件参数以相应数据手册为准。
