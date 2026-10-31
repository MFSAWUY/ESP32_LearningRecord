# 56 HTTP和MQTT设备控制面板

[返回总目录](../../README.md)

## 主机、设备与服务器的分工

前置：第34、35章。完整程序：[network_tool.py](../../examples/python/network_tool.py)、[dashboard.py](../../examples/python/dashboard.py)。HTTP适合请求当前状态和执行明确命令，MQTT适合设备持续发布遥测。局域网内的ESP32访问电脑时，localhost指ESP32自己，因此broker必须监听允许访问的接口，并使用电脑的局域网地址。

dashboard浏览器界面只监听127.0.0.1，浏览器访问电脑本地服务，再由Python访问固定的ESP32地址。令牌从环境变量读取，浏览器不会拿到设备令牌。设备侧仍是教学用HTTP，仅适合受信任的隔离局域网；公共网络需要TLS和完整身份管理。远程SSH服务器的127.0.0.1属于服务器，若需要本地浏览器查看，可用显式SSH端口转发，而不是直接绑定0.0.0.0。

## HTTP接口和操作

[低压控制项目](../../examples/projects/low_voltage_controller/)提供GET /api/status和POST /api/led?on=0或1。写操作要求Authorization: Bearer令牌；空令牌会禁用控制。状态里的led是软件输出命令状态，不是电机实际转动反馈。

Windows：
```powershell
$env:ESP32_TOKEN = '替换为与设备一致的至少16字符随机令牌'
.\.venv\Scripts\python.exe examples/python/dashboard.py --device http://192.168.1.50
.\.venv\Scripts\python.exe examples/python/network_tool.py http http://192.168.1.50/api/status
```
Linux：
```bash
export ESP32_TOKEN='替换为与设备一致的至少16字符随机令牌'
.venv/bin/python examples/python/dashboard.py --device http://192.168.1.50
.venv/bin/python examples/python/network_tool.py http http://192.168.1.50/api/status
```
将IP替换为设备串口输出，然后打开http://127.0.0.1:8080。Python使用固定目标，拒绝把浏览器任意URL当作代理目标；控制请求要求自定义头且不开放CORS。每2秒读取状态，网络失败显示错误而不是假装LED已关闭。

## MQTT操作与回调

两端分别设置MQTT_USERNAME和MQTT_PASSWORD环境变量，Windows用`$env:变量='值'`，Bash用`export 变量='值'`。运行：
```text
python examples/python/network_tool.py mqtt --host 192.168.1.10 --topic "handbook/+/telemetry" --duration 60
```
这里python指对应虚拟环境解释器。发布时加`--publish '{"command":"status"}'`，并将topic改成没有+/#的具体主题。命令是否被设备实现由固件协议决定，broker收到消息不代表执行完成。

Paho版本固定2.1.0，采用CallbackAPIVersion.VERSION2。on_connect中重新订阅，以覆盖自动重连。loop_start运行网络循环；主线程只负责运行时长。TLS启用系统CA校验，未使用跳过证书检查的方式。QoS1允许重复消息，执行器命令应增加命令ID、去重及业务确认。

验收包括正确令牌成功、错误令牌401、参数错误400、设备断网显示502、MQTT断开后恢复。练习：加入“命令编号和确认时间”。参考答案：确认消息应由设备执行逻辑产生，带命令ID及结果，不能由发送端乐观生成。来源：[Paho Python](https://eclipse.dev/paho/files/paho.mqtt.python/html/)、[Mosquitto配置](https://mosquitto.org/man/mosquitto-conf-5.html)。下一步：第57章。

> 验证状态以[验证报告](../../validation/REPORT.md)为准。命令中的端口与IP仅是示例，请按实际环境替换。
