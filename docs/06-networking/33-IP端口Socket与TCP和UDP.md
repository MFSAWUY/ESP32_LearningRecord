# 33 IP端口Socket与TCP和UDP

[返回总目录](../../README.md) · 前置：先完成17章及正文注明的基础；下一步：总目录第34章。

## 分层理解与目标

前置32章。本例在ESP32上提供TCP 5000端口和UDP 5001端口回显服务，Windows/Linux上的Python作为客户端。IP标识网络接口，端口用于区分同一主机上的服务，Socket是程序操作这些通信端点的接口。127.0.0.1表示当前这台电脑自己，不能用它从电脑连接另一台ESP32。

TCP提供有序可靠字节流，但不保留应用消息边界；UDP传输独立数据报，保留边界，但应用必须处理丢失、重复和乱序。TCP的确认说明对方协议栈收到了数据，不一定表示控制任务已经执行成功；需要执行结果时仍应定义应用层ACK。

## 源码数据流

完成Wi-Fi连接后创建TCP监听和UDP本地端口。server.accept接受一条TCP连接，peer.read最多读128字节并原样写回。它一次只服务一个活跃TCP客户端，空闲5秒关闭连接，避免长期无效连接占用资源；不要据此把它宣传为高并发服务器。

UDP使用parsePacket获得报文长度，读取到有界缓冲，再回发到remoteIP与remotePort。超过128字节的报文丢弃并记录，不能把被截断的报文当成完整合法命令。断网时关闭Socket，恢复后重新监听，避免把“Wi-Fi重连”与“业务会话恢复”混为一谈。

## 双平台客户端实验

电脑与ESP32处于允许互访的同一局域网。把下面脚本保存为tcp_check.py，修改HOST，Windows用python tcp_check.py，Linux用python3 tcp_check.py执行，不需要第三方Python包：

```python
import socket
HOST = "192.168.1.123"  # 替换成ESP32实际IP
message = b"hello esp32\n"
with socket.create_connection((HOST, 5000), timeout=3) as sock:
    sock.settimeout(3)
    sock.sendall(message)
    received = bytearray()
    while len(received) < len(message):
        part = sock.recv(len(message) - len(received))
        if not part:
            raise RuntimeError("connection closed before complete echo")
        received.extend(part)
    assert bytes(received) == message
    print("TCP echo verified:", received)
```

循环recv是必要的，不能假定一次recv(n)一定返回n字节。UDP客户端使用socket.socket(socket.AF_INET,socket.SOCK_DGRAM)，settimeout(3)，sendto(message,(HOST,5001))及recvfrom(128)，并核对返回来源；也可用第56章network_tool.py。

## 吞吐与延迟推导

若每条有效载荷20字节、每秒50条，业务数据率为1000B/s，但链路实际开销还包含各层头部、确认、重传和无线管理。RTT是请求到回复的往返时间，不等于单向延迟的一半，除非假设两方向路径及处理近似对称。测延迟应记录多个样本的中位数和尾部，而不是只报最好一次。

## 验收与扩展

正常请求收到完全相同字节；错误端口应超时或拒绝；断网不应使开发板崩溃。TCP若需要多条命令，采用长度前缀或换行分帧并限制最大长度。练习：UDP报文加入序号和应用ACK，统计尝试次数、成功次数与超时；接收方去重但对重复包仍发送ACK，这能修复“ACK丢了导致发送方重试”的情况。重试次数必须有上限。

## 完整源码与双平台操作

完整工程：[examples/arduino/tcp_udp](../../examples/arduino/tcp_udp/README.md)；逐行源码：[main.cpp](../../examples/arduino/tcp_udp/src/main.cpp)；依赖与编译配置：[platformio.ini](../../examples/arduino/tcp_udp/platformio.ini)。示例固件在ESP32上运行，Windows/Linux负责构建、烧录和监视。主线为经典ESP32-D0WD、4 MB Flash；不假定存在PSRAM或可编程板载LED。

1. 按第04/05章完成工具链安装，在手册根目录打开PlatformIO终端；确认所选环境为esp32dev。
2. 断电按本章接线，检查3.3 V逻辑和共地。只接本例所需部件，避免其他实验复用同一引脚。
3. 如有include/config.example.h，复制为config.local.h并调整自己的配置。联网项目必须修改Wi-Fi凭据；无真实凭据时仍可编译，但不能连接你的网络。
4. 两个平台均执行以下构建与设备枚举命令：

```text
pio run -d examples/arduino/tcp_udp
pio device list
```

Windows PowerShell（枚举后输入实际端口）：

```powershell
pio device list
$port = Read-Host '输入枚举出的实际串口'
pio run -d examples/arduino/tcp_udp -t upload --upload-port $port
pio device monitor --port $port --baud 115200
```

Linux Bash（将设备路径换成实际枚举结果）：

```bash
pio device list
read -r -p '输入枚举出的实际串口: ' PORT
pio run -d examples/arduino/tcp_udp -t upload --upload-port "$PORT"
pio device monitor --port "$PORT" --baud 115200
```

烧录前关闭其他串口监视器。Windows检查CH340驱动和端口占用；Linux检查用户组/udev与文件权限，不用sudo运行整个开发环境，也不把设备改为777权限。按本章预期现象记录实际结果，错误分支也要验证，不能用编译成功代替电路和运行验收。

## 验证状态与来源

代码为本手册原创教学实现，所用框架和第三方库遵循其各自许可证。版本固定在工程配置中；外部教程的API不能跨版本直接混用。
静态阅读与构建、桌面运行、真机运行分别记录，最新实际证据见[validation](../../validation/)。本章没有宣称已经完成OLED、传感器或其他真机测试。

参考：[官方接口或器件资料](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/lwip.html)。核对日期：2026-10-29。章节中的推导和例题为教学推导；实际器件参数以相应数据手册为准。
