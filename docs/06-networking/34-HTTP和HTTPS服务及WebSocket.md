# 34 HTTP和HTTPS服务及WebSocket

[返回总目录](../../README.md) · 前置：先完成17章及正文注明的基础；下一步：总目录第35章。

## 三种交互方式

前置32、33章。HTTP以请求和响应交换资源，HTTPS是在TLS保护下传输HTTP，WebSocket通过升级握手建立可双向持续发送消息的通道。WebSocket支持ws和基于TLS的wss；本地演示使用HTTP80与ws81，不将它们说成已经加密。

示例只需现有板卡，配置Wi-Fi后浏览器访问http://设备IP/status，应得到包含uptime_ms的JSON；不存在的路径返回404。HTTP方法同样是接口的一部分，读取状态用GET，改变状态通常应使用具有明确认证和语义的POST/PUT，不能只凭URL名称判断安全。

## HTTP服务代码

WebServer server(80)设置监听端口，server.on注册路径和方法的处理函数，server.send设置状态码、内容类型和正文。loop持续调用handleClient，才能处理新请求。若在同一loop执行很长的传感器等待、TLS握手或文件写入，会增加响应延迟；生产系统可以使用任务队列、缓存状态或异步架构。

响应使用application/json，客户端应先检查状态码和Content-Type，再解析数据。返回200但正文表示错误会让客户端难以区分；400表示请求问题，404表示资源不存在，500系列表示服务端错误。不要在调试响应中回显Wi-Fi密码或证书私钥。

## WebSocket消息与浏览器验证

本例WebSocketsServer监听81端口，接收不超过128字节的文本消息并回显；过大消息关闭连接。浏览器打开开发者工具控制台执行：

```javascript
const ws = new WebSocket('ws://192.168.1.123:81'); // 修改IP
ws.onmessage = event => console.log('echo:', event.data);
ws.onopen = () => ws.send('hello');
ws.onclose = event => console.log('closed', event.code);
```

从HTTPS网页发起ws可能被浏览器作为混合内容阻止，这不是设备Wi-Fi故障；使用合适的本地测试页面或安全部署的wss。频繁推送应控制速率并处理慢客户端，不能无限缓冲。断连后客户端需退避重连，重新订阅应用状态。

## HTTPS客户端与证书验证

工程提供tls_config.example.h。只有复制为tls_config.h并填入真实HTTPS_URL和其证书链对应的ROOT_CA后，HTTPS请求分支才启用。默认没有可信服务地址和CA，因此不会发出伪装成安全测试的请求。先配置SNTP并确认时钟有效，再用WiFiClientSecure.setCACert加载CA，设置连接/读取超时，使用HTTPClient发起GET。

证书验证同时涉及信任链、域名和有效期。把域名URL改成IP可能造成主机名不匹配；设备时间错误会让有效证书被判过期或尚未生效。示例不使用setInsecure绕过验证。需要自建TLS服务器时，使用自己的CA签发测试证书并把CA明确装入设备，而非下载未知证书字符串。

## 验收和练习

HTTP状态与404路径、WebSocket正常回显和超长消息关闭分别验收。HTTPS启用后记录服务端HTTP状态，随后测试错误CA或错误域名应失败；不要只验证成功路径。TLS一次请求在本例中可能短时阻塞HTTP/WebSocket服务，因此本例用于学习功能，后续46章把网络客户端工作移入独立任务。

练习：把uptime_ms同时广播给WebSocket客户端，限制每秒一次；答案要点是在loop用定时状态发送，不在回调里无限循环。若加入控制接口，先定义身份验证、幂等命令和失联行为。

## 完整源码与双平台操作

完整工程：[examples/arduino/http](../../examples/arduino/http/README.md)；逐行源码：[main.cpp](../../examples/arduino/http/src/main.cpp)；依赖与编译配置：[platformio.ini](../../examples/arduino/http/platformio.ini)。示例固件在ESP32上运行，Windows/Linux负责构建、烧录和监视。主线为经典ESP32-D0WD、4 MB Flash；不假定存在PSRAM或可编程板载LED。

1. 按第04/05章完成工具链安装，在手册根目录打开PlatformIO终端；确认所选环境为esp32dev。
2. 断电按本章接线，检查3.3 V逻辑和共地。只接本例所需部件，避免其他实验复用同一引脚。
3. 如有include/config.example.h，复制为config.local.h并调整自己的配置。联网项目必须修改Wi-Fi凭据；无真实凭据时仍可编译，但不能连接你的网络。
4. 两个平台均执行以下构建与设备枚举命令：

```text
pio run -d examples/arduino/http
pio device list
```

Windows PowerShell（枚举后输入实际端口）：

```powershell
pio device list
$port = Read-Host '输入枚举出的实际串口'
pio run -d examples/arduino/http -t upload --upload-port $port
pio device monitor --port $port --baud 115200
```

Linux Bash（将设备路径换成实际枚举结果）：

```bash
pio device list
read -r -p '输入枚举出的实际串口: ' PORT
pio run -d examples/arduino/http -t upload --upload-port "$PORT"
pio device monitor --port "$PORT" --baud 115200
```

烧录前关闭其他串口监视器。Windows检查CH340驱动和端口占用；Linux检查用户组/udev与文件权限，不用sudo运行整个开发环境，也不把设备改为777权限。按本章预期现象记录实际结果，错误分支也要验证，不能用编译成功代替电路和运行验收。

## 验证状态与来源

代码为本手册原创教学实现，所用框架和第三方库遵循其各自许可证。版本固定在工程配置中；外部教程的API不能跨版本直接混用。
静态阅读与构建、桌面运行、真机运行分别记录，最新实际证据见[validation](../../validation/)。本章没有宣称已经完成OLED、传感器或其他真机测试。

参考：[官方接口或器件资料](https://github.com/Links2004/arduinoWebSockets/tree/2.6.1)。核对日期：2026-10-29。章节中的推导和例题为教学推导；实际器件参数以相应数据手册为准。
