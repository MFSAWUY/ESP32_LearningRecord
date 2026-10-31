# Python工具：Windows与Linux

要求Python 3.10及以上；本次具体测试版本见[验证报告](../../validation/REPORT.md)。从手册根目录创建虚拟环境，不修改系统Python或Conda。

Windows：
```powershell
py -3 -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r examples/python/requirements.txt
.\.venv\Scripts\python.exe -m unittest discover -s examples/python/tests -v
```
Linux：
```bash
python3 -m venv .venv
.venv/bin/python -m pip install -r examples/python/requirements.txt
.venv/bin/python -m unittest discover -s examples/python/tests -v
```

下面命令中的python替换成上述虚拟环境解释器。所有脚本支持--help。

| 程序 | 示例与用途 |
|---|---|
| serial_tool.py | `python examples/python/serial_tool.py --ports` 枚举端口；`--port 实际端口 --duration 60 --output data/readings.csv`采集 |
| plot_csv.py | `python examples/python/plot_csv.py data/readings.csv --output data/temperature.png`；`--watch`启用实时查看 |
| network_tool.py | `python examples/python/network_tool.py http http://设备IP/api/status`；mqtt子命令见--help |
| dashboard.py | `python examples/python/dashboard.py --device http://设备IP`，先设置ESP32_TOKEN；只监听127.0.0.1 |
| replay_server.py | `python examples/python/replay_server.py --fail-every 3`；显式模拟HTTP503，并非真实测量 |
| math_lab.py | `python examples/python/math_lab.py rc --plot data/rc.png`；另有filter、alias、pid、power |

串口JSON协议：单行UTF-8对象，LF结束，4096字节上限。建议字段为seq、uptime_ms、temperature_c、humidity_pct、valid；错误读数用null。额外字段保留在raw_json中。主机接收时间不是设备精确采样时间。

远程Ubuntu没有图形桌面时使用绘图的--output模式。串口号必须属于运行Python的那台电脑；开发板接在Windows上，远程Ubuntu不会自动出现对应USB设备。软件回环loop://可用于跨平台检查，不代替USB硬件测试。

API来源：pySerial、Paho MQTT、Matplotlib、Python标准库。脚本为本手册原创教学实现。完整讲解见[总目录](../../README.md)第54–57章。
