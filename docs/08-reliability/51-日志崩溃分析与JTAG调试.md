# 51 日志崩溃分析与JTAG调试

[返回总目录](../../README.md)

串口先枚举再选择；本章端口变量的初始化见[Windows/Linux公共命令](../../examples/esp-idf/README.md)。

## 用证据复现错误

前置：第17、50章。配套工程：[diagnostics](../../examples/esp-idf/diagnostics/)、[gpio](../../examples/esp-idf/gpio/)。日志应包含时间、模块、事件、错误码和必要状态，不输出密码或大量高频原始数据。INFO记录关键状态变化，WARN表示可恢复异常，ERROR表示操作失败；错误级别不是越高越能解决问题。

启动时打印芯片、IDF版本和reset_reason，有助区分主动重启、深睡唤醒、看门狗和欠压。崩溃现场需要保存完整panic信息、回溯地址及对应ELF。用另一版固件ELF解析可能给出错误行号，因此固件bin、ELF和源码版本应一起归档。

## 从panic到源码

idf.py monitor可按构建环境解析回溯。Windows在工程目录运行idf.py -p $serialPort monitor，Linux对应 `idf.py -p "$serial_port" monitor`；如果使用独立工具，需要指定本次构建的ELF和匹配架构的addr2line。先看第一次异常，后续输出可能只是错误扩散。

LoadProhibited常见于非法读取指针，StoreProhibited常见于非法写入；栈溢出、损坏返回地址或不正确的跨任务生命周期也可能让表象不同。结合异常地址、函数调用链和输入数据定位，不能仅凭错误名字认定一个原因。优化会内联函数或重排代码，调试构建更容易理解，但也可能改变时序。

## JTAG需要额外硬件

你现有CH340是串口桥，不是JTAG调试器。经典ESP32通常需要外部兼容调试探针，按板卡和工具文档连接TMS/TCK/TDI/TDO及GND；相关引脚可能与外设和启动配置复用。探针参考电压必须匹配目标电压，避免同时给板卡施加不兼容电源。

安装对应驱动、OpenOCD和GDB后，在VS Code官方IDF扩展中选择正确目标配置；先验证能连接、暂停、读取寄存器，再设置断点。运行中硬断点和软件断点有数量及存储限制，具体由目标支持决定，不随意承诺“任何位置无限断点”。

## 调试会改变系统

暂停CPU后外设、电机、网络超时和另一个核可能表现不同。对控制负载先设置安全状态，用LED验证。竞态故障常在打断点后消失，此时使用时间戳、有限环形日志、逻辑分析仪和事件跟踪比单步更合适。打印也占用CPU和串口带宽，115200 8N1每秒最多约11520字节，超过后会阻塞或丢弃。

验收：保存一次可控错误的日志，匹配正确ELF定位到函数，再修复并用相同输入复测。不要为了演示崩溃让真实执行器保持危险状态。练习：设计长度受限的环形日志，说明覆盖旧数据与停止记录各有什么代价。来源：[IDF致命错误](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/fatal-errors.html)、[JTAG指南](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/jtag-debugging/)。下一步：第52章。
