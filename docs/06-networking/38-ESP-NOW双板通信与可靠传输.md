# 38 ESP-NOW双板通信与可靠传输

[返回总目录](../../README.md) · 前置：先完成17章及正文注明的基础；下一步：总目录第39章。

## 所需硬件与角色

本章需要第二块支持相应ESP-NOW功能的ESP32；现有单板可以编译和查看MAC，但不能完成双端通信验收。无需路由器。两块板的config.local.h使用相同RADIO_CHANNEL，一块ROLE_SENDER=1，另一块ROLE_SENDER=0，分别烧录并记录USB端口与MAC。不要因为两个程序都上传成功，就把角色配置视为已经正确。

## 信道与协议边界

ESP-NOW使用Wi-Fi射频进行设备间通信，不经过普通TCP/IP连接。双方需在相容信道上工作；与路由器STA并存时通常受连接AP的信道约束，不能让两个模块分别“自由选择信道”。本示例断开STA连接并固定信道，后续网关项目再处理与Wi-Fi共存的条件。

广播便于发现接收者，但不等于私密或授权通信。本示例不启用加密，仅限可控实验环境；面向控制设备应使用已知对端、适当密钥和应用身份校验。MAC地址用于链路标识，不是不可伪造的用户身份。

## 固定字节格式

本例每帧7字节：0为0xA5魔数，1为版本1，2为类型(0数据、1ACK)，3～6为大端32位序号。用固定字节协议避免直接发送C结构体时的填充、字节序和编译器布局问题。解析先检查长度、魔数和版本，再取字段，不能在长度不足时访问数组。

```text
偏移       0     1      2       3 4 5 6
内容      A5    01   kind    sequence(big-endian)
```

seq=(b3<<24)|(b4<<16)|(b5<<8)|b6，移位前转换为uint32_t，防止提升为有符号类型后出现边界问题。序号允许应用关联请求与响应；长期运行到回绕时需定义比较窗口，不能简单用新序号永远大于旧序号判断新旧。

## 回调与ACK处理

接收回调在Wi-Fi任务上下文执行，将有效短帧及来源MAC拷贝到容量8的队列，主循环消费；不能在回调里阻塞等待对方应答。它不是硬件ISR，因此使用xQueueSend而非FromISR版本。队列满时本例丢弃，生产版本应统计丢弃次数并按负载设计容量。

接收端收到数据后添加发送方为对端，再单播回对应序号ACK；发送端只有收到匹配当前待确认序号的应用ACK才报告成功和RTT。esp_now_send返回ESP_OK仅说明发送请求被接受，不证明对方应用已经处理。500ms未收到ACK输出timeout；当前基础实验只报告超时，下一轮发送新序号，不自动保证送达。

## 从检测失败到可靠业务

若需要有限重试，应保持相同业务序号重发，设最多3次和退避；接收端按发送方及序号去重，重复消息不重复执行，但仍回ACK。若ACK丢失，接收端可能已执行成功，所以发送端超时不等于业务从未发生。只靠无线重试无法实现永不丢失：掉电恢复还需持久化和应用状态对账。

## 验收与练习

双端同时在线应看到同序号数据及ACK；关闭接收板应出现超时；改成不同信道应无法正常通信。以N次尝试、成功K次估计成功率K/N，需说明测试距离、障碍和环境，不能给未经测试的距离保证。练习：加入上述有界重试和去重，主动延迟或丢弃一个ACK，验证接收方只执行一次但仍能再次确认。

## 完整源码与双平台操作

完整工程：[examples/arduino/espnow](../../examples/arduino/espnow/README.md)；逐行源码：[main.cpp](../../examples/arduino/espnow/src/main.cpp)；依赖与编译配置：[platformio.ini](../../examples/arduino/espnow/platformio.ini)。示例固件在ESP32上运行，Windows/Linux负责构建、烧录和监视。主线为经典ESP32-D0WD、4 MB Flash；不假定存在PSRAM或可编程板载LED。

1. 按第04/05章完成工具链安装，在手册根目录打开PlatformIO终端；确认所选环境为esp32dev。
2. 断电按本章接线，检查3.3 V逻辑和共地。只接本例所需部件，避免其他实验复用同一引脚。
3. 如有include/config.example.h，复制为config.local.h并调整自己的配置。联网项目必须修改Wi-Fi凭据；无真实凭据时仍可编译，但不能连接你的网络。
4. 两个平台均执行以下构建与设备枚举命令：

```text
pio run -d examples/arduino/espnow
pio device list
```

Windows PowerShell（枚举后输入实际端口）：

```powershell
pio device list
$port = Read-Host '输入枚举出的实际串口'
pio run -d examples/arduino/espnow -t upload --upload-port $port
pio device monitor --port $port --baud 115200
```

Linux Bash（将设备路径换成实际枚举结果）：

```bash
pio device list
read -r -p '输入枚举出的实际串口: ' PORT
pio run -d examples/arduino/espnow -t upload --upload-port "$PORT"
pio device monitor --port "$PORT" --baud 115200
```

烧录前关闭其他串口监视器。Windows检查CH340驱动和端口占用；Linux检查用户组/udev与文件权限，不用sudo运行整个开发环境，也不把设备改为777权限。按本章预期现象记录实际结果，错误分支也要验证，不能用编译成功代替电路和运行验收。

## 验证状态与来源

代码为本手册原创教学实现，所用框架和第三方库遵循其各自许可证。版本固定在工程配置中；外部教程的API不能跨版本直接混用。
静态阅读与构建、桌面运行、真机运行分别记录，最新实际证据见[validation](../../validation/)。本章没有宣称已经完成OLED、传感器或其他真机测试。

参考：[官方接口或器件资料](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/espnow.html)。核对日期：2026-10-29。章节中的推导和例题为教学推导；实际器件参数以相应数据手册为准。
