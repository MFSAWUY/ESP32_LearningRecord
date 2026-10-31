# language_basics：C++、有界串口命令与非阻塞采样

适用：经典 ESP32，4 MB Flash，CH340 USB 转串口。只需开发板和数据线；不需要接任何 GPIO。版本固定见 [platformio.ini](platformio.ini)。本例代码为本手册原创教学实现，没有复制第三方示例源码。

**所有 sample 数值都是确定性生成的模拟值，不是 ADC 或传感器实测。** 单位字段用于演示数据建模，程序不会读取外设来伪装真实测量。

## 文件职责

| 文件 | 内容 |
|---|---|
| [include/model.h](include/model.h) | Reading 数据结构、运行状态、统计器与参数解析接口 |
| [src/model.cpp](src/model.cpp) | 无 Arduino 依赖的数值统计和严格周期解析 |
| [src/main.cpp](src/main.cpp) | 串口字节流、命令分帧、周期调度和芯片信息 |

学习顺序：先读 `setup()` 与 `loop()`，再读 `read_commands()` 如何把字节变成一行，最后追踪 `handle_command()` 和模型接口。各行具有注释，完整讲解见手册第 12–17 章。

## Windows PowerShell

在 PlatformIO Core CLI 中，从本例目录执行：

```powershell
pio run
pio device list
$serialPort = Read-Host '输入实际串口'
pio run --target upload --upload-port $serialPort
pio device monitor --port $serialPort --baud 115200
```

## Linux Bash

```bash
pio run
pio device list
read -r -p '输入实际串口: ' SERIAL_PORT
pio run --target upload --upload-port "$SERIAL_PORT"
pio device monitor --port "$SERIAL_PORT" --baud 115200
```

Arduino 固件在 ESP32 上运行，以上命令在电脑执行。Windows 需要对应 CH340 驱动，Linux 需要设备的读写权限；详见安装章节。烧录前关闭占用端口的监视器或 Python 程序。

## 协议与期望输出

串口使用 115200、默认 8N1，输入以 LF 或 CRLF 结束，命令区分大小写。命令正文只接受可打印 ASCII；包含 NUL 等二进制字节的行会整行丢弃。

| 命令 | 行为 |
|---|---|
| `help` | 打印命令说明 |
| `status` | 输出模式、周期、计数、最小最大与均值 |
| `pause` | 停止周期输出，命令仍响应 |
| `resume` | 恢复采样，重新计时，不补发历史采样 |
| `reset` | 清空统计并重置模拟序列，保留运行/暂停模式 |
| `period N` | 仅接受十进制数字 50–60000，单位 ms |

启动输出含 `source=synthetic`、芯片信息和帮助。默认每秒输出一行，例如：

```text
sample,1250,1000,source=synthetic
sample,2250,1100,source=synthetic
```

时间戳取决于实际启动与调度，不保证与示例逐字相同。模拟值依次为 1000、1100、…、1900 mV，然后循环。统计均值使用整数累计和再转换浮点，避免每次读取输出时修改原状态。

## 异常验收

1. 发送 `pause` 后输入 `status`，仍应立即得到响应；发送 `resume` 恢复。
2. 依次发送 `period -1`、`period 0`、`period 100x` 和很大的数字，均应拒绝且不改变原周期。
3. 发送 `period 50` 和 `period 60000`，接受边界值；测试大周期时可用 `status` 验证，无需误判为卡死。
4. 发送超过 63 字符的一整行，再发送 `status`；超长行被完整丢弃，后一行正常。
5. LF 与 CRLF 的行为一致。没有换行时命令尚未提交。
6. 调整统计逻辑前用 `pause`、`reset`，再恢复观察首个样本时最小值等于最大值。

计时采用无符号差值处理一次计数回绕；程序应持续运行并定期检查，不支持跨多次回绕的时间推断。此示例未实现多任务共享统计；要从中断或其他任务调用统计器，必须先加入适当同步或使用队列。

## 可选：在电脑上测试纯 C++ 模型

`model.cpp` 不调用 Arduino API，可以单独在 Windows 或 Linux 编译测试。测试代码在 [tests/test_model.cpp](tests/test_model.cpp)，覆盖合法边界、非法输入、配置不被错误命令修改、无效样本、负数、超过 32 位的累计和与重置。它不测试 UART 驱动或真实时间行为。

安装本机 C++ 工具链和 CMake 后，从此目录执行：

```text
cmake -S . -B .pio/native-tests/cmake
cmake --build .pio/native-tests/cmake --config Release
ctest --test-dir .pio/native-tests/cmake -C Release --output-on-failure
```

Windows 可使用 Visual Studio C++ Build Tools 的 Developer PowerShell，Linux 可用 GCC 与 CMake。此处 `CMakeLists.txt` 仅生成电脑上的测试程序，**不是 ESP-IDF 项目**；ESP32 固件仍通过 `pio run` 构建。测试没有依赖 `assert()`，Release 模式也不会跳过检查。

验证状态以仓库 [validation](../../../validation/) 中的实际记录为准；本文的期望现象与验收步骤不是已经完成的真机测试声明。来源 API：[Arduino-ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/)。
