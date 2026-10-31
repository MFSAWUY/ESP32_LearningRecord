# 原生 ESP-IDF 工程的公共操作

本目录每个子目录都是独立 ESP-IDF 6.1 工程。先按第04/05章安装并激活 IDF 环境，在所选工程目录执行。`idf.py set-target esp32` 用于第一次配置；它会影响已有配置，不要在每次构建前重复执行。

Windows PowerShell：

```powershell
idf.py --version
idf.py set-target esp32
idf.py build
python -m serial.tools.list_ports -v
$serialPort = Read-Host '输入枚举出的实际串口'
idf.py -p $serialPort flash monitor
```

Linux Bash：

```bash
idf.py --version
idf.py set-target esp32
idf.py build
python -m serial.tools.list_ports -v
read -r -p '输入枚举出的实际串口: ' serial_port
idf.py -p "$serial_port" flash monitor
```

串口变量只在当前终端中有效。先关闭占用同一端口的监视器，IDF监视器用 `Ctrl+]` 退出。远程 Ubuntu 只能枚举实际连接到那台 Ubuntu 的 USB 串口；开发板只接本地 Windows 时，应在本地烧录，或按第05章复制编译产物后使用正确偏移。SSH 本身不会转发 USB。

进入各工程 README 查看接线、`menuconfig` 参数与验收现象。仅运行 `idf.py build` 不需要连接开发板，也不会烧录。原生构建与真机状态见[验证报告](../../validation/REPORT.md)。
