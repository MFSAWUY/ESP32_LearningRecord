# 内存与运行状态诊断

经典 ESP32 / 4 MB Flash / ESP-IDF 6.1，无PSRAM依赖。对应手册第50、51章。

## 接线、参数与代码讲解

无需额外外设。串口115200。启动记录IDF版本、CPU核数和复位原因；每2秒输出运行毫秒数、当前空闲堆、历史最低空闲堆、最大连续空闲块和当前任务栈高水位。

`free`下降不一定泄漏，首次建立框架缓存也会下降；`largest`远小于总空闲可能表示碎片。ESP-IDF任务栈接口的单位按IDF约定理解，不直接套用其他FreeRTOS移植的字单位。堆完整性检查失败会断言，以尽早暴露破坏现场。

成功判据：连续运行时日志持续，时间递增，稳定负载下内存指标趋稳。复位时先看reset_reason与第一处错误，保留与固件匹配的ELF。此程序没有故意触发崩溃，也没有替代JTAG硬件。

练习：在另一个实验分支做有限次数的分配/释放，比较总空闲和最大块；不要通过无限分配来验证正常工程。

## 从打开工程到运行

1. 在VS Code或终端打开本工程目录，先按第04/05章激活ESP-IDF 6.1环境。
2. 按[Windows PowerShell与Linux Bash完整命令](../README.md)首次设置esp32目标、构建，再枚举串口并输入实际端口。构建本身不要求连接开发板。
3. 按本页接线，烧录后以115200波特率观察日志；关闭其他占用同一串口的软件。
4. 核对上述预期现象与错误路径，并填写真机验收表。预期日志是教学说明，不是真机运行证据。

源码入口为[main/main.c](main/main.c)，组件依赖见[main/CMakeLists.txt](main/CMakeLists.txt)，默认目标和Flash参数见[sdkconfig.defaults](sdkconfig.defaults)。根CMake启用MINIMAL_BUILD，只构建main及声明的依赖；依赖缺失应修正组件清单，不靠无关组件偶然传递头文件。

## 来源与验证

教学代码为本手册原创，采用[MIT](../../../LICENSE)；ESP-IDF及其组件保持各自许可证。API参考[ESP-IDF v6.1官方文档](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/)。实际编译状态见[验证报告](../../../validation/REPORT.md)，物理实验填[硬件验收表](../../../validation/HARDWARE_CHECKLIST.md)。
