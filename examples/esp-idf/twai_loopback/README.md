# TWAI 新驱动回环

经典 ESP32 / 4 MB Flash / ESP-IDF 6.1，无PSRAM依赖。对应手册第65章。

## 接线、参数与代码讲解

单机实验用短杜邦线连接GPIO25（TX）与GPIO26（RX），共地。代码启用自测试与回环、125kbit/s，不需要第二个节点ACK。这里是控制器逻辑信号回环；接实际CAN总线时必须使用兼容收发器及端接，不能直接把GPIO接CANH/CANL。

`app_main`创建8项接收队列和TWAI节点，注册接收回调；发送标准ID0x123、两个字节的数据，其中第二字节递增。发送帧和缓冲在完成之前持续存在，最多等待1000ms；超时停用节点，避免继续改写待发送内存。

接收中断回调复制帧数据入队并返回是否需要调度更高优先级任务，不做串口格式化。主循环从队列取值后打印`id=123 size=2 counter=...`。序号为8bit，255后回到0是本例预期。

无回环时检查跳线和模式；总线实验另需关闭实验回环、检查波特率、收发器、ACK节点和终端。此工程不实现CANopen或汽车诊断协议。

练习：改变ID并按ID过滤接收；解释控制器硬件过滤与应用层丢弃的CPU开销差异。

## 从打开工程到运行

1. 在VS Code或终端打开本工程目录，先按第04/05章激活ESP-IDF 6.1环境。
2. 按[Windows PowerShell与Linux Bash完整命令](../README.md)首次设置esp32目标、构建，再枚举串口并输入实际端口。构建本身不要求连接开发板。
3. 按本页接线，烧录后以115200波特率观察日志；关闭其他占用同一串口的软件。
4. 核对上述预期现象与错误路径，并填写真机验收表。预期日志是教学说明，不是真机运行证据。

源码入口为[main/main.c](main/main.c)，组件依赖见[main/CMakeLists.txt](main/CMakeLists.txt)，默认目标和Flash参数见[sdkconfig.defaults](sdkconfig.defaults)。根CMake启用MINIMAL_BUILD，只构建main及声明的依赖；依赖缺失应修正组件清单，不靠无关组件偶然传递头文件。

## 来源与验证

教学代码为本手册原创，采用[MIT](../../../LICENSE)；ESP-IDF及其组件保持各自许可证。API参考[ESP-IDF v6.1官方文档](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/)。实际编译状态见[验证报告](../../../validation/REPORT.md)，物理实验填[硬件验收表](../../../validation/HARDWARE_CHECKLIST.md)。
