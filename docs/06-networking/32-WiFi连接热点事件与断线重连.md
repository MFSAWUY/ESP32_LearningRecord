# 32 WiFi连接热点事件与断线重连

[返回总目录](../../README.md) · 前置：先完成17章及正文注明的基础；下一步：总目录第33章。

## 前置、目标与模式

前置18章串口与20章非阻塞编程。只需现有ESP32与USB线，并准备2.4GHz Wi-Fi网络。经典ESP32不因路由器同时支持5GHz就能够连接5GHz；应确认SSID所在频段。STA模式是ESP32作为客户端加入路由器，AP模式是ESP32建立热点供其他设备连接，AP+STA同时启用时两者共享射频资源，并非两套独立无线网卡。

本例目标是连接网络、打印IP与RSSI，并在路由器短暂关闭后恢复；连接成功不代表互联网、DNS或目标服务器都可用。网络排错应按“关联→获得IP→路由/DNS→TCP连接→应用响应”逐层验证。

## 配置与源码组织

首次复制include/config.example.h为include/config.local.h，再修改后者中的WIFI_SSID与WIFI_PASSWORD，保留config.example.h为无真实凭据的模板。名称区分大小写，字符串中的反斜线和引号需要C++转义。只有确实存在的2.4GHz网络和正确凭据才能连接，示例占位值不会自动发现你的密码。

app_network.h包含可复用的连接服务：startNetwork禁用凭据自动持久化，选择STA并启动连接；serviceNetwork每轮检查状态，连接后只打印一次IP，断开时每15秒尝试重连。它不使用while直到连接成功的无限等待，因此密码错误时串口和其他逻辑仍能运行。

```cpp
bool connected = serviceNetwork();
if (connected) { /* 只有此时才启动依赖网络的操作 */ }
```

本地辅助文件特意命名app_network.h，而非network.h。Windows大小写不敏感，network.h会与框架的Network.h发生遮蔽，导致看似无关的WiFi类型编译错误。跨平台项目应让自定义文件名具有明确前缀，不能仅靠大小写区别公共头文件。

## 重连、事件与时间

连接是异步状态变化，WiFi.begin返回不表示已获得IP。简单实验通过轮询状态理解流程，后续可用WiFi.onEvent收集连接、断开和获IP事件。事件回调在其他任务上下文执行，不应直接并发改动普通共享对象；将事件放入队列，在主任务更新业务状态。不要在回调中重新注册回调或长时间阻塞。

本例15秒固定间隔方便观察，产品可采用指数退避T_n=min(T_max,T_0·2^n)并加入随机抖动，避免大批设备同时重连。退避不会修复错误密码，但可控制资源消耗。RSSI的dBm为对数功率表示，P_mW=10^(RSSI/10)；例如−60dBm比−70dBm功率高10倍，但吞吐量还取决于干扰、重传、调制和协议开销。

## 实验与验收

连接后记录IP和RSSI；暂时关闭AP约20秒，观察connected变为0和重试日志，再恢复AP，验证再次获得IP。仅按EN重启一次成功不能证明重连逻辑工作。若始终断开，先查频段、凭据和路由器接入限制，再查供电；Wi-Fi发射电流变化可能暴露供电不足。若电脑和ESP32同SSID却不能通信，还需检查AP客户端隔离或访客网络限制。

## 练习与参考答案

记录断线次数及本次离线持续时间，并在OLED显示。答案要点是只在状态边沿计数，不能每次loop都累加；离线期间保持其他功能，恢复连接后重新建立失效的应用会话，而不是假定旧Socket仍有效。AP配网的完整实现见39章。

## 完整源码与双平台操作

完整工程：[examples/arduino/wifi](../../examples/arduino/wifi/README.md)；逐行源码：[main.cpp](../../examples/arduino/wifi/src/main.cpp)；依赖与编译配置：[platformio.ini](../../examples/arduino/wifi/platformio.ini)。示例固件在ESP32上运行，Windows/Linux负责构建、烧录和监视。主线为经典ESP32-D0WD、4 MB Flash；不假定存在PSRAM或可编程板载LED。

1. 按第04/05章完成工具链安装，在手册根目录打开PlatformIO终端；确认所选环境为esp32dev。
2. 断电按本章接线，检查3.3 V逻辑和共地。只接本例所需部件，避免其他实验复用同一引脚。
3. 如有include/config.example.h，复制为config.local.h并调整自己的配置。联网项目必须修改Wi-Fi凭据；无真实凭据时仍可编译，但不能连接你的网络。
4. 两个平台均执行以下构建与设备枚举命令：

```text
pio run -d examples/arduino/wifi
pio device list
```

Windows PowerShell（枚举后输入实际端口）：

```powershell
pio device list
$port = Read-Host '输入枚举出的实际串口'
pio run -d examples/arduino/wifi -t upload --upload-port $port
pio device monitor --port $port --baud 115200
```

Linux Bash（将设备路径换成实际枚举结果）：

```bash
pio device list
read -r -p '输入枚举出的实际串口: ' PORT
pio run -d examples/arduino/wifi -t upload --upload-port "$PORT"
pio device monitor --port "$PORT" --baud 115200
```

烧录前关闭其他串口监视器。Windows检查CH340驱动和端口占用；Linux检查用户组/udev与文件权限，不用sudo运行整个开发环境，也不把设备改为777权限。按本章预期现象记录实际结果，错误分支也要验证，不能用编译成功代替电路和运行验收。

## 验证状态与来源

代码为本手册原创教学实现，所用框架和第三方库遵循其各自许可证。版本固定在工程配置中；外部教程的API不能跨版本直接混用。
静态阅读与构建、桌面运行、真机运行分别记录，最新实际证据见[validation](../../validation/)。本章没有宣称已经完成OLED、传感器或其他真机测试。

参考：[官方接口或器件资料](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html)。核对日期：2026-10-29。章节中的推导和例题为教学推导；实际器件参数以相应数据手册为准。
