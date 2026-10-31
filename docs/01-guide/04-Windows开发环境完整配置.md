# 04 Windows 开发环境完整配置

前置：第 01–03 章。验证目标：Windows 11 x64。配套：[language_basics](../../examples/arduino/language_basics/)。返回：[总目录](../../README.md)。本文的命令供你执行；手册生成不等于已在你的电脑安装工具或烧录设备。

## 工具职责和版本

VS Code 是编辑器；PlatformIO 扩展管理项目并带有自己的 Core/Python 环境；Arduino-ESP32 是芯片框架；交叉编译器将 C++ 变成 Xtensa 机器码；esptool 通过串口将固件写入 Flash。Python 上位机使用另一个项目虚拟环境。已经安装 Anaconda 不意味着这些工具已经安装，也不应把全部工具包塞进 Conda base。

本书固定 `platform = espressif32@8.0.0`，其发布说明列出 Arduino-ESP32 3.3.12；独立 ESP-IDF 路线固定 v6.1。前者的 Arduino 内部 IDF 基础和后者的独立工具链不是同一环境。不要为了“统一版本”手动替换 PlatformIO 下载的框架文件。[发布依据](https://github.com/platformio/platform-espressif32/releases/tag/v8.0.0)

## 一、安装与路径

1. 安装 Windows x64 版 [VS Code](https://code.visualstudio.com/download) 和 [Git](https://git-scm.com/downloads/win)，在终端检查 `code --version`、`git --version`。
2. 在 VS Code 扩展页查找并安装发布者 PlatformIO 的 **PlatformIO IDE**。首次启动需下载 Core 和依赖，等待安装结束再创建项目。
3. 打开本书一个示例文件夹，而不是先新建空白 `.cpp`。在 PlatformIO 图标中打开 **Core CLI**，它会提供正确的 `pio` 命令环境。
4. 如需纯命令行方式，可在独立虚拟环境安装 PlatformIO；不要和扩展自动管理的环境相互覆盖。本文主线使用扩展自带 CLI。

项目、IDF 和工具路径使用不含空格的简短路径，例如 `D:\esp\projects`、`D:\esp\idf-tools`。当前手册位置可供阅读和 Arduino 构建；若某工具不能处理特殊字符，复制单个工程到上述简短路径测试，保留原始资料。ESP-IDF 官方明确要求其项目与 IDF 路径不含空格。用户名含空格时尤其应显式设置安装位置。[路径要求](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/windows-start-project.html)

## 二、识别 CH340 串口

拔下开发板，打开设备管理器，再插入数据线，观察“端口（COM 和 LPT）”新增设备。没有端口先换数据线、USB 口并检查未知设备；确认使用 CH340 后，从 [WCH 官方驱动页](https://www.wch.cn/downloads/CH341SER_ZIP.html) 获取对应驱动，按安装器步骤操作并重新插拔。不要从不明网盘获取驱动。

在 PlatformIO Core CLI 执行：

```powershell
pio --version
pio device list
```

记下实际出现的串口；以下使用输入变量，避免把某个 COM 号当固定配置。串口监视器、Python 脚本与烧录器不能同时独占同一端口，烧录前关闭其他占用程序。

## 三、构建、烧录、监视

在仓库根目录打开 PlatformIO Core CLI：

```powershell
Set-Location .\examples\arduino\language_basics
pio run
$serialPort = Read-Host '输入 pio device list 中的实际串口'
pio run --target upload --upload-port $serialPort
pio device monitor --port $serialPort --baud 115200
```

`pio run` 在没有开发板时也能编译；上传需要设备；监视显示设备输出。退出监视通常使用 `Ctrl+C`。若长时间停在连接阶段，先确认端口，再尝试按住 BOOT、开始上传、连接后松开，必要时短按 EN；不同兼容板的自动下载电路可能不同。

打开 `platformio.ini`：

```ini
[env:esp32dev]
platform = espressif32@8.0.0
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 115200
```

`board` 是编译与默认 Flash 参数集合，不证明每个 GPIO 的板上位置。`monitor_speed` 与 `Serial.begin()` 一致。保守的上传速率便于首次排错，成功后可以再尝试提高。用 `pio pkg list` 记录真正安装的包版本；如果平台下载失败，保留错误文本，检查网络和官方仓库可达性，不静默换到旧核心。

## 四、Python 上位机的独立环境

在仓库根目录使用 Python 3.10 以上版本，推荐项目选择一个固定小版本记录：

```powershell
python --version
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install --upgrade pip
```

每个 Python 示例按其 README 或依赖文件安装依赖。直接调用 `.venv` 中的解释器可以避开 PowerShell 激活脚本执行策略问题，不需要全局降低执行策略。安装后用 `python -m pip` 的形式保证 pip 对应正确解释器，而不是依赖 PATH 中另一个 `pip.exe`。

## 五、进入 ESP-IDF 阶段

从 [EIM 官方入口](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html) 安装管理器，选择 ESP-IDF v6.1 和目标 `esp32`，设置无空格路径。安装 VS Code 的 Espressif IDF 扩展，选择已有 EIM 环境。通过 EIM 的 **Open IDF Terminal** 进入激活终端，检查 `idf.py --version`，再打开一个 ESP-IDF 示例执行：

```powershell
idf.py set-target esp32
idf.py build
$serialPort = Read-Host '输入实际串口'
idf.py -p $serialPort flash monitor
```

`set-target` 会重新配置工程，应在新建工程时使用，不要在已有自定义配置上反复无意识执行。IDF 监视器通常用 `Ctrl+]` 退出。具体工程需要的 `menuconfig` 参数见其 README。

## Windows 长路径的定位与处理

下载完成后仍出现找不到编译器子程序、创建文件失败或路径过长时，先检查错误中的完整路径。平台缓存、框架库和项目构建目录叠加后可能超过部分 Windows 工具的路径处理能力。优先把工程与 PlatformIO 缓存设到短目录；不要因为 `cc1plus` 启动失败就立即修改 C++ 源码。

如果必须保留当前仓库位置，可以在确认盘符未占用后，把它临时映射到一个短盘符。下面是**可选排错步骤**，不是每次安装的必需步骤。在同一个 PowerShell 中，先用 `subst` 查看现有映射、用 `Get-PSDrive -PSProvider FileSystem` 查看磁盘；确认 P 盘未使用后：

```powershell
subst P: 'D:\VSCode_data\C++\ESP32\handbook'
$env:PLATFORMIO_CORE_DIR = 'P:\.tools\platformio'
Set-Location P:\examples\arduino\language_basics
pio run
```

此方法创建的是盘符映射，不会搬动源码；`PLATFORMIO_CORE_DIR` 只影响当前进程及其子进程，可能触发在新目录下载工具。不要同时打开使用旧缓存与新缓存的两个构建。故障排查结束后先退出该盘上的进程并切换回原路径，再执行 `subst P: /D` 解除映射；该命令解除映射，不删除目录。正常安装优先选择永久短路径。真实构建记录以 [validation](../../validation/) 为准，不依据示例命令推断测试已经通过。

## 验收与排障

完成标准是：能显示工具版本、枚举设备、编译 `language_basics`、上传后看到递增的周期计数。头文件的红色波浪线可能是索引尚未完成，应以真实构建结果为准。`Access denied` 优先查串口占用；乱码查波特率；Python 找不到模块查解释器路径；下载失败查网络及源地址，不卸载所有工具重来。

练习：同一条 `python` 命令为什么可能在两个终端运行不同版本？答案：PATH、Conda 激活、venv 和工具专用终端改变了解释器解析顺序。保留 `Get-Command python` 和 `python -c "import sys; print(sys.executable)"` 输出可验证。下一步：[06 电路定律](../02-math-and-physics/06-电压电流电阻功率与基本电路定律.md)，也可先做第 18 章首次串口实验。
