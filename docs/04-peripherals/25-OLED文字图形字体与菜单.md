# 25 OLED文字图形字体与菜单

[返回总目录](../../README.md) · 前置：先完成17章及正文注明的基础；下一步：总目录第26章。

## 设备识别与先决条件

先完成24章，确认OLED的7位地址。0.96寸描述物理尺寸，不能确定控制器、分辨率或引脚顺序。config.example.h默认SSD1306、128×64、0x3C；若确认是SH1106，修改OLED_CONTROLLER=1106；SSD1306的128×32变体修改OLED_HEIGHT=32。SH1106本例仅提供128×64配置，不把任意宏组合都宣称支持。

## 像素、显存与刷新预算

单色像素仅需1bit，128×64屏全帧数据量为128×64/8=1024字节。若用I²C传输，每字节还占ACK位，单纯数据在100kHz下至少需要1024×9/100000≈92.2ms，还没有包含地址、命令及软件开销。400kHz时理想数据部分约23ms，因此不能承诺100kHz下实现每秒60次全帧刷新。

屏幕常按页组织显存，SSD1306每页8行；驱动库负责把像素坐标转换成页字节及位操作。SH1106内部列布局不同，错误驱动可能造成偏移或花屏，即使扫描地址完全相同。F型U8g2构造器使用全帧缓冲，方便先画整帧再发送；1/2页缓冲可减少RAM，但要求在分页循环中重新绘图。

## 代码逐步说明

首先用Wire检查地址ACK；失败时输出明确错误，不假装显示成功。display.setI2CAddress(OLED_ADDRESS<<1)执行U8g2所需的地址格式转换。begin初始化控制器；setBusClock设置总线速度。每轮clearBuffer清空内存，drawStr写入文字，drawFrame画空框，drawBox填充，sendBuffer才真正传到屏幕。

```cpp
display.clearBuffer();
display.setFont(u8g2_font_6x12_tf);
display.drawStr(0, 12, "ESP32 handbook");
display.sendBuffer();
```

drawStr的y坐标是字体基线，不总是字符左上角。字体决定哪些字符可显示，UTF-8字符串不保证当前字体有中文字形。加入中文时选择覆盖对应字符的字体并使用正确UTF-8接口，或生成精简字库；完整中文字库会增加Flash占用。不要把空白字形误判成I²C故障。

## 菜单设计方法

显示逻辑与输入事件分离：GPIO去抖产生NEXT、SELECT事件；菜单状态保存当前页面和选项；渲染函数只根据状态画图，不在绘图函数中等待按键。只在状态改变或定时刷新到期时重绘可减少总线负载。正在测量的值与最后有效值要区分，读取失败时显示ERR或过期标记，而不是长期保留数字让人以为仍有效。

## 验收与练习

看到标题、逐秒运行时间和移动条形；拔掉屏幕后重新上电应看到无ACK日志。黑屏依次检查供电、地址、控制器、分辨率和复位要求。练习：增加两页菜单，第一页运行时间、第二页空闲堆；使用GPIO27短按切页。参考实现应沿用19章去抖状态机，并把选中页保存为枚举，不能在每次原始低电平都切页。

## 完整源码与双平台操作

完整工程：[examples/arduino/oled](../../examples/arduino/oled/README.md)；逐行源码：[main.cpp](../../examples/arduino/oled/src/main.cpp)；依赖与编译配置：[platformio.ini](../../examples/arduino/oled/platformio.ini)。示例固件在ESP32上运行，Windows/Linux负责构建、烧录和监视。主线为经典ESP32-D0WD、4 MB Flash；不假定存在PSRAM或可编程板载LED。

1. 按第04/05章完成工具链安装，在手册根目录打开PlatformIO终端；确认所选环境为esp32dev。
2. 断电按本章接线，检查3.3 V逻辑和共地。只接本例所需部件，避免其他实验复用同一引脚。
3. 如有include/config.example.h，复制为config.local.h并调整自己的配置。联网项目必须修改Wi-Fi凭据；无真实凭据时仍可编译，但不能连接你的网络。
4. 两个平台均执行以下构建与设备枚举命令：

```text
pio run -d examples/arduino/oled
pio device list
```

Windows PowerShell（枚举后输入实际端口）：

```powershell
pio device list
$port = Read-Host '输入枚举出的实际串口'
pio run -d examples/arduino/oled -t upload --upload-port $port
pio device monitor --port $port --baud 115200
```

Linux Bash（将设备路径换成实际枚举结果）：

```bash
pio device list
read -r -p '输入枚举出的实际串口: ' PORT
pio run -d examples/arduino/oled -t upload --upload-port "$PORT"
pio device monitor --port "$PORT" --baud 115200
```

烧录前关闭其他串口监视器。Windows检查CH340驱动和端口占用；Linux检查用户组/udev与文件权限，不用sudo运行整个开发环境，也不把设备改为777权限。按本章预期现象记录实际结果，错误分支也要验证，不能用编译成功代替电路和运行验收。

## 验证状态与来源

代码为本手册原创教学实现，所用框架和第三方库遵循其各自许可证。版本固定在工程配置中；外部教程的API不能跨版本直接混用。
静态阅读与构建、桌面运行、真机运行分别记录，最新实际证据见[validation](../../validation/)。本章没有宣称已经完成OLED、传感器或其他真机测试。

参考：[官方接口或器件资料](https://github.com/olikraus/u8g2/wiki/u8g2setupcpp)。核对日期：2026-10-29。章节中的推导和例题为教学推导；实际器件参数以相应数据手册为准。
