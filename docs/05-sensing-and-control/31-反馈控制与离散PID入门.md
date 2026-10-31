# 31 反馈控制与离散PID入门

[返回总目录](../../README.md) · 前置：先完成17章及正文注明的基础；下一步：总目录第32章。

## 本例是可重复的模型实验

前置09、20、29章。本例在ESP32上计算一阶被控对象模型，并通过串口输出CSV，不直接驱动电机，也不将模型值写成实测传感器数据。这样可以先理解控制律，再把真实测量、执行器限制和机械系统接入；调参失败不会导致实体负载失控。

## 从目标到反馈

目标r、测量y，误差e=r−y。开环只按预设输入操作，不能补偿负载变化；闭环将实际输出送回控制器。比例项Kp·e推动系统接近目标；积分项Ki∫e dt积累持续偏差；微分项Kd·de/dt响应变化速度。各增益的量纲随输入输出单位改变，秒与毫秒混用会让Ki、Kd相差千倍。

## 离散公式与数值例子

采样周期Δt=0.02s，积分I[k]=I[k−1]+e[k]Δt。微分采用对测量求导D[k]=−(y[k]−y[k−1])/Δt，避免设定值突然改变造成微分冲击。控制候选u*=Kp·e+Ki·I+Kd·D，输出限定0≤u≤2。

若e=1、Kp=1.2、Ki=0.8、初始积分0，第一步积分候选为0.02，比例和积分产生1.216。微分初值取前一测量等于当前初始测量，避免人为初始跳变。有限差分对噪声敏感，真实系统通常需要微分滤波；增加Kd可能同时放大测量噪声。

## 对象模型与积分饱和

模型τdy/dt=u−y，取τ=0.5s，前向欧拉得到y[k+1]=y[k]+Δt(u[k]−y[k])/τ。Δt相对τ足够小是本例数值行为稳定的前提；减小采样间隔并不自动改善真实系统，因为采样噪声、运算延迟和执行器带宽也有影响。

输出饱和时误差还可能持续存在，若积分无限增加，目标恢复后也难以快速退出饱和。本例条件积分：输出候选在范围内则接受积分；已经超过上限且误差为负时允许回退；低于下限且误差为正时允许恢复。它是简单抗饱和方法，不是适合所有对象的通用最优控制器。

## 实验和读图

运行后记录simulation_time_s,target,model_y,control_u。观察模型输出逐渐接近1，改变Kp/Ki/Kd后比较上升时间、超调和稳定误差。模拟时间按固定dt推进，打印或调度变慢只会让实验墙钟时间变长；实际控制必须用确定采样周期或正确处理实际dt。

## 练习与参考答案

把目标改为无法达到的3，观察u上限2和积分限制，再将目标恢复1。对照移除抗饱和条件后的恢复过程。进一步加入固定扰动或一拍测量延迟，但在CSV列名中明确model。连接真实电机前必须新增急停、传感器失效处理、输出限速、驱动电流限制及安全方向验证，不能把仿真调参结果直接当作硬件安全保证。

## 完整源码与双平台操作

完整工程：[examples/arduino/pid](../../examples/arduino/pid/README.md)；逐行源码：[main.cpp](../../examples/arduino/pid/src/main.cpp)；依赖与编译配置：[platformio.ini](../../examples/arduino/pid/platformio.ini)。示例固件在ESP32上运行，Windows/Linux负责构建、烧录和监视。主线为经典ESP32-D0WD、4 MB Flash；不假定存在PSRAM或可编程板载LED。

1. 按第04/05章完成工具链安装，在手册根目录打开PlatformIO终端；确认所选环境为esp32dev。
2. 断电按本章接线，检查3.3 V逻辑和共地。只接本例所需部件，避免其他实验复用同一引脚。
3. 如有include/config.example.h，复制为config.local.h并调整自己的配置。联网项目必须修改Wi-Fi凭据；无真实凭据时仍可编译，但不能连接你的网络。
4. 两个平台均执行以下构建与设备枚举命令：

```text
pio run -d examples/arduino/pid
pio device list
```

Windows PowerShell（枚举后输入实际端口）：

```powershell
pio device list
$port = Read-Host '输入枚举出的实际串口'
pio run -d examples/arduino/pid -t upload --upload-port $port
pio device monitor --port $port --baud 115200
```

Linux Bash（将设备路径换成实际枚举结果）：

```bash
pio device list
read -r -p '输入枚举出的实际串口: ' PORT
pio run -d examples/arduino/pid -t upload --upload-port "$PORT"
pio device monitor --port "$PORT" --baud 115200
```

烧录前关闭其他串口监视器。Windows检查CH340驱动和端口占用；Linux检查用户组/udev与文件权限，不用sudo运行整个开发环境，也不把设备改为777权限。按本章预期现象记录实际结果，错误分支也要验证，不能用编译成功代替电路和运行验收。

## 验证状态与来源

代码为本手册原创教学实现，所用框架和第三方库遵循其各自许可证。版本固定在工程配置中；外部教程的API不能跨版本直接混用。
静态阅读与构建、桌面运行、真机运行分别记录，最新实际证据见[validation](../../validation/)。本章没有宣称已经完成OLED、传感器或其他真机测试。

参考：[官方接口或器件资料](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/esp_timer.html)。核对日期：2026-10-29。章节中的推导和例题为教学推导；实际器件参数以相应数据手册为准。
