# 62 ESP-NOW无线采集网络与网关

[返回总目录](../../README.md)

## 拓扑与协议

前置：第23、38、44、54章。完整工程：[espnow_gateway](../../examples/projects/espnow_gateway/)。需要两块ESP32；节点连接SHT30，网关用USB接电脑。网关先输出串口JSON，电脑可再转发到数据库或MQTT。这种USB网关避免同一无线电同时连接AP造成信道切换；后续做Wi-Fi网关必须统一AP与节点信道。

两个设备使用相同ESPNOW_CHANNEL。消息定义固定字节布局，不直接发送C结构体：第0字节版本1，第1字节类型1数据/2确认，第2至5字节为小端uint32序号，第6至7字节为小端int16摄氏度百分之一。无效传感器读数使用保留值−32768，接收端转换为null，不冒充0℃。

例如23.50℃编码整数2350=0x092E，在线路中低字节0x2E先发。量化误差不超过约0.005℃，但传感器本身准确度通常更低；增加传输小数位不提高真实测量精度。节点MAC与序号构成一次启动内的来源和次序，正式系统还应增加boot/session ID避免重启后序号冲突。

## 构建两个角色

Windows在同一工程分别执行：
```powershell
pio device list
$gatewayPort = Read-Host '输入网关串口'
$nodePort = Read-Host '输入节点串口'
pio run -e esp32dev -t upload --upload-port $gatewayPort
pio run -e node -t upload --upload-port $nodePort
pio device monitor --port $gatewayPort --baud 115200
```
Linux：
```bash
pio device list
read -r -p '输入网关串口: ' gateway_port
read -r -p '输入节点串口: ' node_port
pio run -e esp32dev -t upload --upload-port "$gateway_port"
pio run -e node -t upload --upload-port "$node_port"
pio device monitor --port "$gateway_port" --baud 115200
```
esp32dev是网关，node通过编译宏选择节点角色；端口必须通过拔插和枚举确认。SHT30接节点3V3/GND/21/22。config.local.h两端设同一信道，默认不连接Wi-Fi AP。

## 接收回调为什么要短

ESP-NOW接收回调在Wi-Fi任务上下文执行，不是硬件ISR。回调只检查长度/版本，复制固定大小消息入队，耗时打印与ACK发送放在loop。队列满时非阻塞提交会失败，这是有限缓冲的明确丢弃策略，不能用无限队列掩盖处理能力不足。以每节点3秒一条估算负载，同时考虑瞬时碰撞而不只平均速率。

节点发送后等待相同序号的业务ACK，500ms超时输出错误。本教学项目不自动重发，便于先观察丢包；网关允许重复遥测，未实现持久去重。广播和ACK在此示例中未加密认证，只适合非敏感教学遥测，不能直接用于门锁或危险执行器。

## 验收、排错与扩展

网关收到JSON，节点看到ACK和往返时间；关闭网关后节点应超时；重开恢复；拔传感器后valid=false。无数据先核对角色、信道、供电、距离，再查队列与错误码。GPIO接线与无线连接是两条独立链路，串口日志应能区分二者。

练习：加入最多3次重试。参考：同一消息保持原序号，接收端按MAC+启动ID+序号去重，重复消息仍回ACK；使用随机退避减轻多个节点同步重发。来源：[ESP-NOW官方文档](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/network/esp_now.html)。下一步：第63章。

> 验证状态以[验证报告](../../validation/REPORT.md)为准。命令中的端口与IP仅是示例，请按实际环境替换。
