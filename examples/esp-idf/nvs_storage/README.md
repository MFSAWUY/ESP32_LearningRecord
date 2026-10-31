# NVS 持久启动计数

经典 ESP32 / 4 MB Flash / ESP-IDF 6.1，无PSRAM依赖。对应手册第47章。

## 接线、参数与代码讲解

无需额外外设。代码使用默认NVS分区、命名空间`handbook`、键`boots`。首次缺失键按0处理，增加后写入并调用`nvs_commit`；达到UINT32_MAX保持饱和值，不回绕。

预期第一次输出`persistent boot count=1`，按EN重启后计数递增，断电后重新供电仍保留。实际值取决于分区已有内容。应用退出前关闭句柄。

初始化、读取或提交失败均记录错误。本例不会因为NVS初始化异常自动擦除整块分区，先保存配置并检查分区与版本；全片擦除会丢失计数。写入后未commit的行为不能当成持久性保证。

练习：增加一项有上下限的采样周期。参考思路是读取缺失键时使用默认值，读取成功后仍校验范围，写入成功再commit，只在用户修改时持久化，避免高频擦写。

## 从打开工程到运行

1. 在VS Code或终端打开本工程目录，先按第04/05章激活ESP-IDF 6.1环境。
2. 按[Windows PowerShell与Linux Bash完整命令](../README.md)首次设置esp32目标、构建，再枚举串口并输入实际端口。构建本身不要求连接开发板。
3. 按本页接线，烧录后以115200波特率观察日志；关闭其他占用同一串口的软件。
4. 核对上述预期现象与错误路径，并填写真机验收表。预期日志是教学说明，不是真机运行证据。

源码入口为[main/main.c](main/main.c)，组件依赖见[main/CMakeLists.txt](main/CMakeLists.txt)，默认目标和Flash参数见[sdkconfig.defaults](sdkconfig.defaults)。根CMake启用MINIMAL_BUILD，只构建main及声明的依赖；依赖缺失应修正组件清单，不靠无关组件偶然传递头文件。

## 来源与验证

教学代码为本手册原创，采用[MIT](../../../LICENSE)；ESP-IDF及其组件保持各自许可证。API参考[ESP-IDF v6.1官方文档](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/)。实际编译状态见[验证报告](../../../validation/REPORT.md)，物理实验填[硬件验收表](../../../validation/HARDWARE_CHECKLIST.md)。
