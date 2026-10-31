# 05 Linux 开发环境完整配置

前置：第 01–03 章。主线发行版：Ubuntu 24.04 LTS x86_64。配套：[language_basics](../../examples/arduino/language_basics/)。返回：[总目录](../../README.md)。其他发行版的包名、串口组可能不同，以实际系统为准。

## 工具安装与环境边界

安装 VS Code 官方 Linux 包，在扩展页安装 **PlatformIO IDE**。第一次启动等待 Core 和工具链下载完成，通过 PlatformIO 的 **Core CLI** 运行 `pio`。这避免把系统 Python、Conda 和 PlatformIO 私有环境混用。准备基础系统包：

```bash
sudo apt update
sudo apt install git python3 python3-venv python3-pip build-essential
git --version
python3 --version
```

这里 `sudo` 只用于系统软件安装。日常编辑、构建和 Python 包安装在用户环境中进行，不用 `sudo code`、`sudo pio` 或 `sudo pip`。否则生成的文件可能归 root 所有，后续普通用户无法修改，串口权限问题也被掩盖。

Ubuntu 的系统 Python 可能限制全局 pip 安装，这是保护包管理器的一部分。正确做法是在项目根目录创建虚拟环境：

```bash
python3 -m venv .venv
./.venv/bin/python -m pip install --upgrade pip
./.venv/bin/python -c "import sys; print(sys.executable)"
```

后续按 Python 示例依赖文件安装，统一使用 `./.venv/bin/python -m pip`。Shell 激活是便捷选项，不是虚拟环境运行的必要条件。

## USB 串口识别与权限

CH340 通常由内核 `ch341` 驱动处理。先比较插拔前后的设备列表，再检查日志，不预设端口一定叫 `/dev/ttyUSB0`：

```bash
lsusb
ls -l /dev/serial/by-id/ 2>/dev/null
pio device list
```

`/dev/serial/by-id/` 的稳定名字适合重复实验，但设备没有可区分序列号时仍需检查。读内核日志可能需要管理员权限，可按需执行 `sudo dmesg --follow` 并观察一次插拔；不要把完整无关日志作为常规输出。

找到设备后检查权限，例如将下行变量改为实际设备：

```bash
read -r -p '输入实际串口路径: ' SERIAL_PORT
ls -l "$SERIAL_PORT"
id
```

如果设备所属组为 `dialout` 且当前用户不在该组，执行：

```bash
sudo usermod -aG dialout "$USER"
```

随后注销并重新登录，运行 `id` 确认。若需要支持更广泛调试器，可按 [PlatformIO 官方 udev 文档](https://docs.platformio.org/en/latest/core/installation/udev-rules.html) 下载、检查规则文件，再放入 `/etc/udev/rules.d/`、重新加载并重新插拔。先核对 VID/PID 是否匹配；普通 CH340 串口使用正确用户组通常已能工作。不要使用 `chmod 777` 作为持久解决方案，也不要让全部开发工具以 root 身份运行。

## 同一套源码的构建与上传

在仓库根目录打开 PlatformIO Core CLI，执行：

```bash
cd examples/arduino/language_basics
pio --version
pio run
read -r -p '输入 pio device list 中的串口: ' SERIAL_PORT
pio run --target upload --upload-port "$SERIAL_PORT"
pio device monitor --port "$SERIAL_PORT" --baud 115200
```

这里的 C++ 源码和 Windows 相同，仅端口与 Shell 语法不同。`pio run` 负责交叉编译，生成的 ESP32 固件不能作为 Linux 可执行程序直接运行。使用 `Ctrl+C` 退出 PlatformIO 串口监视，再运行会使用同一设备的 Python 程序或上传工具。

工程固定 `platform = espressif32@8.0.0`、`board = esp32dev`、`framework = arduino`。平台发布说明对应 Arduino-ESP32 3.3.12；用 `pio pkg list` 记录实际依赖，不把“已下载”当“已编译”。[版本依据](https://github.com/platformio/platform-espressif32/releases/tag/v8.0.0)

## ESP-IDF 独立环境

根据 [ESP-IDF 官方入门](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html) 安装 EIM，选择 v6.1 和 `esp32` 工具。通过 EIM 启动已激活环境的终端，在 VS Code Espressif IDF 扩展中选择该环境。不要让项目依赖用户碰巧安装的另一个 Python 或 Xtensa 编译器。

在一个完整 IDF 示例目录内：

```bash
idf.py --version
idf.py set-target esp32
idf.py build
read -r -p '输入实际串口: ' SERIAL_PORT
idf.py -p "$SERIAL_PORT" flash monitor
```

退出 IDF 监视器使用 `Ctrl+]`。IDF 安装和工程路径使用无空格目录，如 `$HOME/esp`；若使用引号引用路径，引用只能解决 Shell 拆词，不能改变构建工具本身不支持空格的限制。

## 使用你已配置的远程 Ubuntu

本项目源码位置为 `/home/pc/wty/ESP32/handbook`，测试、虚拟环境与构建缓存放在 `/home/pc/wty/ESP32/test`。本次实际连接的Ubuntu是22.04.5 LTS，文档与CI目标还包含24.04；不能把一台机器通过等同于两个版本均已验证。

Windows上的VS Code可以通过Remote SSH连接已配置的`lab-linux`，再打开上述handbook目录。这样编辑器界面在本机，扩展、终端和构建在远端。也可以只用PowerShell中的`ssh lab-linux`进入Bash。两个环境的Python解释器和串口列表分别独立。

首次传送本手册可在本地根目录执行下面的命令；打包脚本排除工具链、构建缓存与本地凭据。再次同步前保存远端对同名文件的修改，避免两个工作副本互相覆盖。

```powershell
python scripts/package_sources.py --output .tools/source-sync.tar.gz
scp .tools/source-sync.tar.gz lab-linux:/home/pc/wty/ESP32/test/source-sync.tar.gz
ssh lab-linux
```

进入远端Bash后：

```bash
tar -xzf /home/pc/wty/ESP32/test/source-sync.tar.gz -C /home/pc/wty/ESP32/handbook
cd /home/pc/wty/ESP32/handbook
python3 -m venv /home/pc/wty/ESP32/test/desktop-venv
/home/pc/wty/ESP32/test/desktop-venv/bin/python -m pip install -r examples/python/requirements.txt platformio==6.1.18
/home/pc/wty/ESP32/test/desktop-venv/bin/python scripts/check_handbook.py
/home/pc/wty/ESP32/test/desktop-venv/bin/python -m unittest discover -s examples/python/tests -v
export PLATFORMIO_CORE_DIR=/home/pc/wty/ESP32/test/platformio
/home/pc/wty/ESP32/test/desktop-venv/bin/python scripts/build_examples.py --kind arduino --build-root /home/pc/wty/ESP32/test/builds --output /home/pc/wty/ESP32/test/build-linux.json
```

`venv`命令用于首次创建；已经完成设置时直接使用环境，不必重新安装。系统缺少venv时按前述apt步骤安装，或使用明确指定的独立Python解释器创建环境。不要借用Conda已有的绘图、串口等包来掩盖requirements缺项。工具链首次下载体积较大，下载超时要保留日志，不能记录成源码编译通过。

**USB接在哪台电脑，串口就在哪台电脑。** 普通SSH不会自动把Windows的CH340送到Ubuntu。开发板接远端时可在远端正常flash/monitor；接本地时可以本地编译烧录，也可以把远端原生IDF工程的合并镜像复制回来。后一种方式在远端已激活IDF环境、工程构建成功后运行：

```bash
cd /home/pc/wty/ESP32/handbook/examples/esp-idf/gpio
idf.py build
idf.py merge-bin -o merged-binary.bin -f raw
```

随后在本地已激活IDF的PowerShell中：

```powershell
scp lab-linux:/home/pc/wty/ESP32/handbook/examples/esp-idf/gpio/build/merged-binary.bin .
python -m serial.tools.list_ports -v
$serialPort = Read-Host '输入本地开发板的实际串口'
python -m esptool --chip esp32 --port $serialPort write-flash 0x0 merged-binary.bin
```

此处仅适用于刚由本工程生成、默认偏移0x0的RAW合并镜像；单独应用bin不能套用此偏移。合并镜像包含引导程序、分区与应用，填充间隙可能覆盖旧分区数据，实验前保存重要NVS/文件内容。不要把Linux的CMake缓存复制到Windows后直接继续构建。合并选项来源：[IDF 6.1 idf.py参考](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/tools/idf-py.html#merge-binaries-merge-bin)。

## 跨平台细节与故障定位（检查清单）

Linux 文件名区分大小写，`Config.h` 与 `config.h` 是不同文件。源码和 Markdown 使用 UTF-8，Shell 脚本使用 LF 换行，出现 `/bin/bash^M` 通常与 CRLF 有关。运行脚本可显式用 `bash script.sh`；需要直接执行时再按项目说明赋予执行权限。路径在 Python 中使用 `pathlib`，在命令行中始终给含变量的路径加引号。

串口不存在时先查设备和驱动；串口存在但权限不足查所属组；权限正常但设备忙查是否有监视器或串口服务占用。个别桌面辅助服务可能抢占 USB 串口，应根据日志确认具体服务，再做针对性调整，不能无依据禁用所有服务。

验收：编译、烧录、监视三步分别成功；断开设备时程序给出明确错误；重新插入后重新枚举端口能恢复。练习：为什么加了用户组后当前终端仍失败？答案：当前登录会话仍持有旧组列表，需要新登录会话。为什么 `sudo pio` 可用而 `pio` 不可用不代表问题解决？答案：绕过了权限而非修复，并可能产生 root 所有的缓存。下一步：[06 电路定律](../02-math-and-physics/06-电压电流电阻功率与基本电路定律.md)。
