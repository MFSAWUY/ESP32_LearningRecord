# 65 RS485和Modbus及TWAI与CAN

[返回总目录](../../README.md)

串口先枚举再选择；本章端口变量的初始化见[Windows/Linux公共命令](../../examples/esp-idf/README.md)。

## 协议层次与物理连接

前置：第23、43、44章。工程：[rs485_echo](../../examples/esp-idf/rs485_echo/)、[twai_loopback](../../examples/esp-idf/twai_loopback/)。RS485规定电气传输，Modbus RTU规定帧和业务；不能把两者当同义词。TWAI是芯片CAN控制器接口，实际CAN总线还需要收发器，不能把GPIO直接连到CANH/CANL。

RS485示例用3.3V兼容收发器：GPIO17接DI，GPIO16接RO，GPIO27接DE和/RE；A/B接总线。收发器逻辑电平必须与ESP32匹配，不要默认廉价5V模块的RO可安全输入。总线两端按线缆阻抗终端匹配，不在每个节点都并联终端；两只120Ω终端并联，断电时理想测得约60Ω，实际还受偏置网络影响。设备间参考电位及隔离按布线环境设计。

## RS485代码与RTU时序

UART2采用19200、8N1，驱动半双工模式用RTS控制收发方向。收到数据后等待发送方释放总线再回显，发送结束等待硬件排空。echo只是传输实验，不是Modbus从站，不能对任意Modbus请求原样回显并称为响应。

19200下8N1每字节10bit，字符时间10/19200≈0.521ms；3.5字符约1.823ms。但实际Modbus设备可能使用8E1等11bit格式，而且高波特率的间隔要求应按规范采用推荐固定时间，不能机械套8N1算式。完整RTU解析须包含站号、功能码、长度、CRC16和异常响应；CRC错误丢弃，广播地址不返回普通响应。

## CAN仲裁与TWAI回环

经典CAN显性位覆盖隐性位，在同时发送时逐位比较标识符完成仲裁。低数值ID往往具有更高总线优先级，但不能据此忽略帧格式和扩展ID仲裁细节。经典ESP32支持经典CAN帧，不支持CAN FD。位时间包括同步段、传播段及相位段，采样点与时钟误差预算必须结合收发器、线长和目标速率。

示例使用新IDF6.1 esp_twai接口，125kbit/s、自测试及回环，GPIO25/26按工程README测试接线。这个测试不是两节点总线验收。RX回调在ISR上下文，使用xQueueSendFromISR；发送数据缓冲保持有效直到transmit_wait_all_done返回。正常CAN网络应关闭自测试模式、增加收发器、终端及第二个节点来确认ACK和错误恢复。

## 两端运行与验收

Windows与Linux都在工程目录运行idf.py set-target esp32、idf.py build；上传分别使用`idf.py -p $serialPort flash monitor`和`idf.py -p "$serial_port" flash monitor`。先进行单机回环，再接实际总线。RS485与CAN使用不同收发器，不能混接。

练习：总线断开时为什么持续重发可能让系统卡住？参考：控制器会累计错误并可能进入bus-off，应用需要有限等待、状态报告及受控恢复。不要用无限重启循环替代诊断。来源：[IDF TWAI](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/twai.html)、[Modbus规范](https://www.modbus.org/specs.php)。下一步：第66章。

> 编译和真机状态见[验证报告](../../validation/REPORT.md)。额外硬件未连接时，不将理论或软件仿真当作实测。
