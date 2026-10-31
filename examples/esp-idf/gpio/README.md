# GPIO 按键与 LED

经典 ESP32 / 4 MB Flash / ESP-IDF 6.1，无PSRAM依赖。对应手册第40、42章。

## 接线、参数与代码讲解

| ESP32 | 外设 |
|---|---|
| GPIO25 | 330Ω～1kΩ电阻后接LED阳极；阴极接GND |
| GPIO27 | 按键一端；另一端接GND |

输入使用内部上拉，按下为低。GPIO编号不是排针顺序。先只连接LED验证极性，再接按键。

`app_main`配置输入输出；每5ms读取一次，原始值变化时更新`changed`；同一值保持30ms后才更新稳定状态。仅稳定的按下边沿切换LED，松开不切换。改变采样周期会改变响应延迟，改变30ms阈值会改变抗抖能力。

预期日志为`pressed, led=1`与`pressed, led=0`交替。按住2秒只触发一次，连续按10次应有10次切换。随机触发查共地、上拉和接触；LED不亮查极性及限流电阻。初始化失败立即报错，不继续运行错误配置。

练习：加入800ms长按。参考实现逻辑为稳定按下时记时，达到阈值只发一次长按事件；松开清除标记。

## 从打开工程到运行

1. 在VS Code或终端打开本工程目录，先按第04/05章激活ESP-IDF 6.1环境。
2. 按[Windows PowerShell与Linux Bash完整命令](../README.md)首次设置esp32目标、构建，再枚举串口并输入实际端口。构建本身不要求连接开发板。
3. 按本页接线，烧录后以115200波特率观察日志；关闭其他占用同一串口的软件。
4. 核对上述预期现象与错误路径，并填写真机验收表。预期日志是教学说明，不是真机运行证据。

源码入口为[main/main.c](main/main.c)，组件依赖见[main/CMakeLists.txt](main/CMakeLists.txt)，默认目标和Flash参数见[sdkconfig.defaults](sdkconfig.defaults)。根CMake启用MINIMAL_BUILD，只构建main及声明的依赖；依赖缺失应修正组件清单，不靠无关组件偶然传递头文件。

## 来源与验证

教学代码为本手册原创，采用[MIT](../../../LICENSE)；ESP-IDF及其组件保持各自许可证。API参考[ESP-IDF v6.1官方文档](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/)。实际编译状态见[验证报告](../../../validation/REPORT.md)，物理实验填[硬件验收表](../../../validation/HARDWARE_CHECKLIST.md)。
