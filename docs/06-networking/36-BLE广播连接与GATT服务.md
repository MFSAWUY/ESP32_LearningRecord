# 36 BLE广播连接与GATT服务

[返回总目录](../../README.md) · 前置：先完成17章及正文注明的基础；下一步：总目录第37章。

## BLE使用场景与基础模型

前置通信分层和回调概念。BLE适合低功耗传感、手机配置和小数据交互，不是经典蓝牙串口的另一种名称。GAP负责广播、发现和连接等行为；GATT把数据组织为服务、特征和描述符，特征用UUID标识，权限和属性决定可读、可写、可通知等操作。UUID不是访问密码，知道UUID也不应自动拥有控制权限。

本例仅需开发板和具有BLE能力的手机或电脑。使用可浏览GATT的客户端搜索ESP32-Handbook，找到源码中的服务UUID和特征UUID。普通电脑“蓝牙配对”界面不是完整GATT调试器，能发现设备不表示操作系统已经将它映射成串口。

## 工程结构和操作

BLEDevice::init初始化设备名称；createServer创建GATT服务端；createService与createCharacteristic定义数据模型。示例特征同时具有READ、WRITE、NOTIFY属性，初始值为ready。客户端可以读取，也可以写入最多32字节的短文本；订阅通知后约每秒收到一次uptime字符串。

```cpp
value = service->createCharacteristic(
    VALUE, BLECharacteristic::PROPERTY_READ |
           BLECharacteristic::PROPERTY_WRITE |
           BLECharacteristic::PROPERTY_NOTIFY);
```

BLE2902描述符为客户端配置通知提供入口。客户端需要主动启用通知，服务端调用notify不意味着所有已连接客户端都已经订阅。通知不含GATT层确认；Indicate有对应确认过程，吞吐和可靠性行为不同。若需要“设备已执行某命令”的保证，仍应设计应用级结果特征或命令序号。

## MTU与有效负载

默认ATT MTU常见为23字节，典型通知值可用长度是MTU减3，即20字节；实际协商可能更大，不能把大字符串一次发送成功作为所有手机都支持的证据。本例通知很短，超过长度的应用消息需要分包，并明确序号、总长度和重组超时。广播包、连接后的ATT报文和应用值分别有自己的长度限制，不要混为同一缓冲区。

若每秒10条20字节通知，应用数据仅200B/s，但总无线耗能还包含连接事件与协议开销。连接间隔较长通常降低唤醒频率，同时增加交互延迟；不能只凭“BLE”三个字承诺整块开发板达到某个微安值。

## 回调、并发和重连

连接回调与loop可能并发执行。示例对connected和restart标志使用std::atomic<bool>，断开回调只设置标志，loop里重新启动广播。这样可让第二次连接也有明确入口。示例写回调只打印短文本，不做长时间采样或文件操作；复杂业务应通过队列转交任务。

本例用于本地学习，没有配置配对、绑定和加密访问控制，所以不连接危险执行器或存放秘密。实际设备需根据产品模式定义是否允许未配对读写、绑定信息如何恢复、手机更换后如何重新授权，而不是只隐藏设备名。

## 验收与练习

读取ready、写入短文本、订阅通知、主动断开、再次扫描连接五步都应验证。手机缓存旧服务时可能看不到修改后的UUID，重新发现服务或清除相关缓存后再诊断。练习：分成只读传感特征和只写命令特征，命令加入长度检查；答案要点是权限按用途最小化，属性定义不能只在客户端界面约定。

## 完整源码与双平台操作

完整工程：[examples/arduino/ble](../../examples/arduino/ble/README.md)；逐行源码：[main.cpp](../../examples/arduino/ble/src/main.cpp)；依赖与编译配置：[platformio.ini](../../examples/arduino/ble/platformio.ini)。示例固件在ESP32上运行，Windows/Linux负责构建、烧录和监视。主线为经典ESP32-D0WD、4 MB Flash；不假定存在PSRAM或可编程板载LED。

1. 按第04/05章完成工具链安装，在手册根目录打开PlatformIO终端；确认所选环境为esp32dev。
2. 断电按本章接线，检查3.3 V逻辑和共地。只接本例所需部件，避免其他实验复用同一引脚。
3. 如有include/config.example.h，复制为config.local.h并调整自己的配置。联网项目必须修改Wi-Fi凭据；无真实凭据时仍可编译，但不能连接你的网络。
4. 两个平台均执行以下构建与设备枚举命令：

```text
pio run -d examples/arduino/ble
pio device list
```

Windows PowerShell（枚举后输入实际端口）：

```powershell
pio device list
$port = Read-Host '输入枚举出的实际串口'
pio run -d examples/arduino/ble -t upload --upload-port $port
pio device monitor --port $port --baud 115200
```

Linux Bash（将设备路径换成实际枚举结果）：

```bash
pio device list
read -r -p '输入枚举出的实际串口: ' PORT
pio run -d examples/arduino/ble -t upload --upload-port "$PORT"
pio device monitor --port "$PORT" --baud 115200
```

烧录前关闭其他串口监视器。Windows检查CH340驱动和端口占用；Linux检查用户组/udev与文件权限，不用sudo运行整个开发环境，也不把设备改为777权限。按本章预期现象记录实际结果，错误分支也要验证，不能用编译成功代替电路和运行验收。

## 验证状态与来源

代码为本手册原创教学实现，所用框架和第三方库遵循其各自许可证。版本固定在工程配置中；外部教程的API不能跨版本直接混用。
静态阅读与构建、桌面运行、真机运行分别记录，最新实际证据见[validation](../../validation/)。本章没有宣称已经完成OLED、传感器或其他真机测试。

参考：[官方接口或器件资料](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ble.html)。核对日期：2026-10-29。章节中的推导和例题为教学推导；实际器件参数以相应数据手册为准。
