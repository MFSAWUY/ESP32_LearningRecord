# 35 MQTT消息通信与本地服务器

[返回总目录](../../README.md) · 前置：先完成17章及正文注明的基础；下一步：总目录第36章。

## 架构与主题

前置Wi-Fi、TCP和JSON概念。MQTT采用客户端—broker发布订阅结构，ESP32与电脑都连接broker，客户端之间不直接寻找彼此。示例默认设备ID esp32-lab-01，对应lab/esp32-lab-01/telemetry、command和ack三个主题。多个设备必须使用不同client ID，否则新连接可能踢掉旧连接。

QoS0是至多一次，QoS1可能重复投递，QoS2处理协议层恰好一次交付但不自动保证业务副作用恰好一次。当前PubSubClient示例发布为QoS0，订阅请求QoS1，不能声称遥测获得了端到端可靠送达。命令采用ON/OFF而非TOGGLE，使重复投递不会重复翻转输出。

## Windows与Linux准备本地broker

Windows从Mosquitto官方发行包安装并在终端能找到mosquitto、mosquitto_passwd等程序；Ubuntu安装mosquitto和mosquitto-clients。测试时选择独立前台实例或系统服务之一，避免两者同时占用1883。进入本案例目录，把mosquitto.conf.example复制为mosquitto.conf，把acl.example复制为acl，然后执行：

```text
mosquitto_passwd -c passwords esp32
mosquitto -c mosquitto.conf -v
```

-c只在首次创建新密码文件时使用，会覆盖同名文件；后续增加用户不要重复带-c。前台进程的工作目录必须是案例目录，才能找到相对路径的passwords和acl；作为系统服务运行时改为正确绝对路径并赋予服务账号读取权限。Windows按安装目录使用完整可执行文件路径，PowerShell以&调用带空格的路径。

配置listener 1883 0.0.0.0才能允许局域网设备连接，allow_anonymous false配合密码文件和ACL控制访问。只监听localhost时电脑本机测试会成功，ESP32却无法连接。防火墙仅开放本次实验需要的局域网范围，不把明文1883服务转发到公网；密码认证不等于TLS加密。使用你的电脑实际LAN IP填写MQTT_HOST，并检查AP客户端隔离。

## 源码与故障恢复

mqtt.setServer设置broker，setCallback处理命令，setSocketTimeout(3)限制协议读等待，loop维持心跳及收包。连接尝试之间至少间隔5秒，但TCP连接本身仍可能受系统连接超时影响，本例不是严格实时控制循环。断开Wi-Fi或MQTT时LED输出复位LOW，恢复后重新订阅，不依赖旧会话继续有效。

回调的payload不是自动带\0的字符串，因此先检查length，再用memcmp比较精确字节数；超长或未知命令不驱动输出。GPIO25经330Ω接LED，不连接高功率负载。每两秒发布uptime和heap，publish返回值主要反映本地发送操作是否成功，不是云端业务存储确认。

## 实验与验收

另开终端订阅lab/esp32-lab-01/#，并向command发布ON和OFF，验证LED与ack一致；用错误密码验证拒绝连接；停止broker再重启，设备应恢复订阅。mosquitto_sub/mosquitto_pub的-u与-P可用于本地实验账号，但-P明文参数可能出现在进程列表或历史，重要账号应改用更合适的凭据管理。

retained消息会在新订阅时重放，不应随意保留瞬时执行命令。产品可使用状态主题保留当前状态，用命令ID、过期时间和ACK防止旧命令重新执行。练习：重复发送ON十次，输出仍保持ON；再增加命令序号与响应，明确处理超时和重复行为。

## 完整源码与双平台操作

完整工程：[examples/arduino/mqtt](../../examples/arduino/mqtt/README.md)；逐行源码：[main.cpp](../../examples/arduino/mqtt/src/main.cpp)；依赖与编译配置：[platformio.ini](../../examples/arduino/mqtt/platformio.ini)。示例固件在ESP32上运行，Windows/Linux负责构建、烧录和监视。主线为经典ESP32-D0WD、4 MB Flash；不假定存在PSRAM或可编程板载LED。

1. 按第04/05章完成工具链安装，在手册根目录打开PlatformIO终端；确认所选环境为esp32dev。
2. 断电按本章接线，检查3.3 V逻辑和共地。只接本例所需部件，避免其他实验复用同一引脚。
3. 如有include/config.example.h，复制为config.local.h并调整自己的配置。联网项目必须修改Wi-Fi凭据；无真实凭据时仍可编译，但不能连接你的网络。
4. 两个平台均执行以下构建与设备枚举命令：

```text
pio run -d examples/arduino/mqtt
pio device list
```

Windows PowerShell（枚举后输入实际端口）：

```powershell
pio device list
$port = Read-Host '输入枚举出的实际串口'
pio run -d examples/arduino/mqtt -t upload --upload-port $port
pio device monitor --port $port --baud 115200
```

Linux Bash（将设备路径换成实际枚举结果）：

```bash
pio device list
read -r -p '输入枚举出的实际串口: ' PORT
pio run -d examples/arduino/mqtt -t upload --upload-port "$PORT"
pio device monitor --port "$PORT" --baud 115200
```

烧录前关闭其他串口监视器。Windows检查CH340驱动和端口占用；Linux检查用户组/udev与文件权限，不用sudo运行整个开发环境，也不把设备改为777权限。按本章预期现象记录实际结果，错误分支也要验证，不能用编译成功代替电路和运行验收。

## 验证状态与来源

代码为本手册原创教学实现，所用框架和第三方库遵循其各自许可证。版本固定在工程配置中；外部教程的API不能跨版本直接混用。
静态阅读与构建、桌面运行、真机运行分别记录，最新实际证据见[validation](../../validation/)。本章没有宣称已经完成OLED、传感器或其他真机测试。

参考：[官方接口或器件资料](https://mosquitto.org/documentation/authentication-methods/)。核对日期：2026-10-29。章节中的推导和例题为教学推导；实际器件参数以相应数据手册为准。
