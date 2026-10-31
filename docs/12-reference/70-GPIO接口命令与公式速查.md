# 70 GPIO接口命令与公式速查

[返回总目录](../../README.md) · 前置：各实验基础章节 · 下一步：第71章

## 本手册引脚约定

下面是教学工程约定，不是所有开发板的固定定义。各工程可能按资源用途调整，最终以该工程README和源码为准。先核对板上丝印，不能把物理排针序号等同于GPIO。

| 用途 | 推荐GPIO | 必要条件 |
|---|---|---|
| OLED/SHT30 SDA、SCL | 21、22 | 3.3V逻辑，核对模块电源与上拉 |
| 外接LED/PWM | 25 | 串联限流电阻；DAC实验会复用此脚 |
| 按键 | 27 | 接地按下，普通GPIO可使用内部上拉 |
| ADC电位器 | 34 | ADC1；仅输入；无内部上下拉 |
| SPI SCK/MISO/MOSI | 18、19、23 | 外设电平兼容，按工程配置CS |
| 示例SPI CS | 13 | 与JTAG复用时重新规划 |
| UART实验 | 16、17 | 确认实际模组没有把这两脚分配给PSRAM |

经典ESP32注意：GPIO6–11通常连接Flash，不作为普通外设；GPIO34–39仅输入；GPIO0、2、5、12、15涉及启动配置；GPIO1/3常用于下载和串口。用户的Type-C连接CH340串口桥，不因此获得芯片原生USB能力。GPIO并非5V容忍输入。

## 常用命令

| 目标 | 命令 |
|---|---|
| 枚举串口 | `pio device list` |
| Arduino构建 | `pio run -d examples/arduino/serial_info` |
| 上传 | `pio run -t upload --upload-port 实际端口` |
| 监视 | `pio device monitor --port 实际端口 --baud 115200` |
| IDF选择芯片 | `idf.py set-target esp32`，首次或切换目标使用 |
| IDF配置 | `idf.py menuconfig` |
| IDF构建 | `idf.py build` |
| IDF上传监视 | `idf.py -p 实际端口 flash monitor` |
| Python测试 | `python -m unittest discover -s examples/python/tests -v` |
| 手册结构检查 | `python scripts/check_handbook.py` |

Windows端口如COM7，Linux如/dev/ttyUSB0，只能用枚举后确认的值。Windows虚拟环境解释器为`.venv\Scripts\python.exe`，Linux为`.venv/bin/python`。IDF命令在已激活的IDF环境使用，切换target会重置相关构建配置，不能每次无脑执行。

## 公式、单位与适用边界

| 关系 | 公式 | 约束 |
|---|---|---|
| 欧姆定律 | U=IR | 线性电阻，V/A/Ω |
| LED限流 | R=(Vs−Vf)/I | 考虑Vf范围及电阻功率 |
| 电阻功率 | P=UI=I²R | W，核对器件额定功率 |
| RC时间常数 | τ=RC | Ω×F=s |
| RC阶跃充电 | Vc=Vs(1−e^(−t/RC)) | 初始电压0，理想一阶模型 |
| I²C上升时间 | tr≈0.8473RpCb | 30%到70%阈值的理想RC近似 |
| 理想量化步长 | Δ=Vspan/2^N | 不等同真实ESP32 ADC校准公式 |
| UART 8N1吞吐上限 | 字节/秒≈baud/10 | 未计协议头和处理停顿 |
| 一阶数字低通 | y←y+α(x−y)，α=1−e^(−dt/τ) | 正dt、τ，按采样假设使用 |
| 同电压侧平均电流 | Iavg=ΣIi·ti/Σti | 不混合不同电源侧电流 |
| 单色帧缓冲 | W×H/8 | 逐像素1bit，不含对象开销 |

不要把速查公式替代完整推导。尤其PWM平均输出、采样定理和电池续航有明确模型条件。完整数值实验：[math_lab.py](../../examples/python/math_lab.py)。

## 速查使用练习

115200波特率8N1发送1024字节，理想最短约1024×10/115200=88.9ms；实际还受帧协议、缓冲和任务调度影响。若要20Hz发送完整1024字节帧，就已经超出此串口净负载能力，应减少数据、提高可靠波特率或换接口。来源：[GPIO官方约束](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/gpio.html)。
