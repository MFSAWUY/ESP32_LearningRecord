# SPI 彩屏分块 DMA 显示

经典 ESP32 / 4 MB Flash / ESP-IDF 6.1，无PSRAM依赖。对应手册第64章。

## 接线、参数与代码讲解

需要240×240 ST7789 SPI屏，不是0.96寸I²C OLED。电源、背光和是否含稳压按实际模块资料确认。

| ESP32 | 屏幕信号 |
|---|---|
| GPIO18 / GPIO23 | SCLK / MOSI（SDA） |
| GPIO13 / GPIO21 | CS / DC |
| GPIO27 / GND | RESET / GND |

GPIO21与默认OLED SDA冲突，本实验单独连接。没有CS的屏幕需按驱动规则把CS配置为−1。没有MISO不影响只写显示。

`app_main`创建SPI总线、面板IO、ST7789面板，然后分配240×16×2=7680字节DMA缓冲。RGB565像素交换高低字节后发送；每批绘制等待完成回调通过任务通知唤醒，避免DMA读取期间修改缓冲。等待1秒超时则停止任务以保留现场。

预期整屏每秒红、绿、蓝循环。颜色异常查RGB/BGR、反色和字节序；位置偏移查模块可视区域。更高帧率需计算传输带宽，20MHz不能提供完整240×240 RGB565的30fps有效像素流。

练习：只刷新一个矩形，比较字节数与刷新时间；在确认前一传输完成后再复用缓冲。

## 从打开工程到运行

1. 在VS Code或终端打开本工程目录，先按第04/05章激活ESP-IDF 6.1环境。
2. 按[Windows PowerShell与Linux Bash完整命令](../README.md)首次设置esp32目标、构建，再枚举串口并输入实际端口。构建本身不要求连接开发板。
3. 按本页接线，烧录后以115200波特率观察日志；关闭其他占用同一串口的软件。
4. 核对上述预期现象与错误路径，并填写真机验收表。预期日志是教学说明，不是真机运行证据。

源码入口为[main/main.c](main/main.c)，组件依赖见[main/CMakeLists.txt](main/CMakeLists.txt)，默认目标和Flash参数见[sdkconfig.defaults](sdkconfig.defaults)。根CMake启用MINIMAL_BUILD，只构建main及声明的依赖；依赖缺失应修正组件清单，不靠无关组件偶然传递头文件。

## 来源与验证

教学代码为本手册原创，采用[MIT](../../../LICENSE)；ESP-IDF及其组件保持各自许可证。API参考[ESP-IDF v6.1官方文档](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/)。实际编译状态见[验证报告](../../../validation/REPORT.md)，物理实验填[硬件验收表](../../../validation/HARDWARE_CHECKLIST.md)。
