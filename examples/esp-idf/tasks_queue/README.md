# 任务、队列与延迟测量

经典 ESP32 / 4 MB Flash / ESP-IDF 6.1，无PSRAM依赖。对应手册第43、44、45、46章。

## 接线、参数与代码讲解

无需额外外设。`sample_t`包含uint32序号和int64微秒时间；生产者优先级5、消费者4，各分配3072字节任务栈，队列容纳8项。

生产者使用`vTaskDelayUntil`维持100ms周期，发送采用零等待；队列满时丢弃最新样本并记录。消费者最多等待1000ms，收到后计算当前时间减生成时间的端到端排队延迟。队列复制值，避免传递局部变量指针导致生命周期错误。

预期`seq`递增、`latency=... us`持续输出。具体延迟是板端实际测量，不能以电脑构建日志推断。任务创建和队列分配失败断言；生产者停止时消费者能报告超时。

练习：消费者增加200ms处理延迟，观察队列先积压再满；8项队列可吸收短突发，却不能长期解决生产速率10Hz大于消费速率5Hz的问题。恢复负载后观察恢复行为。

## 从打开工程到运行

1. 在VS Code或终端打开本工程目录，先按第04/05章激活ESP-IDF 6.1环境。
2. 按[Windows PowerShell与Linux Bash完整命令](../README.md)首次设置esp32目标、构建，再枚举串口并输入实际端口。构建本身不要求连接开发板。
3. 按本页接线，烧录后以115200波特率观察日志；关闭其他占用同一串口的软件。
4. 核对上述预期现象与错误路径，并填写真机验收表。预期日志是教学说明，不是真机运行证据。

源码入口为[main/main.c](main/main.c)，组件依赖见[main/CMakeLists.txt](main/CMakeLists.txt)，默认目标和Flash参数见[sdkconfig.defaults](sdkconfig.defaults)。根CMake启用MINIMAL_BUILD，只构建main及声明的依赖；依赖缺失应修正组件清单，不靠无关组件偶然传递头文件。

## 来源与验证

教学代码为本手册原创，采用[MIT](../../../LICENSE)；ESP-IDF及其组件保持各自许可证。API参考[ESP-IDF v6.1官方文档](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/)。实际编译状态见[验证报告](../../../validation/REPORT.md)，物理实验填[硬件验收表](../../../validation/HARDWARE_CHECKLIST.md)。
