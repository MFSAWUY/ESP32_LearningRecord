# 深睡与定时唤醒

经典 ESP32 / 4 MB Flash / ESP-IDF 6.1，无PSRAM依赖。对应手册第48章。

## 接线、参数与代码讲解

无需额外外设，仅开发板和USB线。测试前保存串口工作，深睡会重启应用执行流程。

`RTC_DATA_ATTR wake_count`放在深睡可保持的RTC内存；启动记录计数与唤醒原因，等待1秒让串口日志发送完，启用10秒定时唤醒后进入深睡。它不是NVS：断电和某些复位会使RTC数据失效。

预期每约11秒出现新一轮启动与`sleep`日志，计数递增。实际周期包含启动耗时。把10秒改为60秒应增大睡眠占比，但不能直接据此声称续航变为六倍。

测量整板供电电流，区分启动、等待和睡眠；CH340、稳压器和电源灯可能主导休眠功耗。不要把电流档表笔并接电源；按第11章串联测量。看到USB端口仍存在不代表ESP32未睡眠。

练习：增加外部唤醒。参考思路是先核对经典ESP32的RTC GPIO能力与唤醒电平，明确外部上拉和睡眠中的电平保持，再引入唤醒源配置。

## 从打开工程到运行

1. 在VS Code或终端打开本工程目录，先按第04/05章激活ESP-IDF 6.1环境。
2. 按[Windows PowerShell与Linux Bash完整命令](../README.md)首次设置esp32目标、构建，再枚举串口并输入实际端口。构建本身不要求连接开发板。
3. 按本页接线，烧录后以115200波特率观察日志；关闭其他占用同一串口的软件。
4. 核对上述预期现象与错误路径，并填写真机验收表。预期日志是教学说明，不是真机运行证据。

源码入口为[main/main.c](main/main.c)，组件依赖见[main/CMakeLists.txt](main/CMakeLists.txt)，默认目标和Flash参数见[sdkconfig.defaults](sdkconfig.defaults)。根CMake启用MINIMAL_BUILD，只构建main及声明的依赖；依赖缺失应修正组件清单，不靠无关组件偶然传递头文件。

## 来源与验证

教学代码为本手册原创，采用[MIT](../../../LICENSE)；ESP-IDF及其组件保持各自许可证。API参考[ESP-IDF v6.1官方文档](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/)。实际编译状态见[验证报告](../../../validation/REPORT.md)，物理实验填[硬件验收表](../../../validation/HARDWARE_CHECKLIST.md)。
