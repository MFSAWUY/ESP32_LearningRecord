# 54 Python串口通信与命令行工具

[返回总目录](../../README.md)

## 从字节流到一条有效测量

前置：第23章UART、第13章字符串。目标是让电脑可靠接收设备数据，而不是假设每次read都会获得一整条消息。串口是字节流，一条消息可能分三次到达，多条消息也可能一次到达。我们约定UTF-8编码，每个JSON对象占一行，以换行符结束；一行最多4096字节。JSON本身不负责串口分帧，换行才是本协议的边界。

对应完整程序：[serial_tool.py](../../examples/python/serial_tool.py)、[protocol.py](../../examples/python/protocol.py)。设备可使用[环境监测工程](../../examples/projects/environment_station/)。打开端口可能通过DTR触发开发板复位，最开始出现启动日志很正常，不能把启动文字硬解析成温度。

合法数据示例：
```json
{"seq":42,"uptime_ms":84000,"valid":true,"temperature_c":23.50,"humidity_pct":48.20}
```
seq标识采集次序，uptime_ms是本次启动后的毫秒数，不是UTC时间。读数失败应输出valid=false以及null。Python的bool是int子类，所以验证整数时必须先排除bool，否则true会被误当作序号1。额外字段保留，允许后续添加节点ID而不破坏旧记录器。

## Windows与Linux操作

从手册根目录执行。首次Windows：
```powershell
py -3 -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r examples/python/requirements.txt
.\.venv\Scripts\python.exe examples/python/serial_tool.py --ports
$serialPort = Read-Host '输入枚举出的实际串口'
.\.venv\Scripts\python.exe examples/python/serial_tool.py --port $serialPort --duration 60 --output data/readings.csv
```
Linux：
```bash
python3 -m venv .venv
.venv/bin/python -m pip install -r examples/python/requirements.txt
.venv/bin/python examples/python/serial_tool.py --ports
read -r -p '输入枚举出的实际串口: ' serial_port
.venv/bin/python examples/python/serial_tool.py --port "$serial_port" --duration 60 --output data/readings.csv
```
端口按枚举结果替换，先关闭PlatformIO监视器。Linux权限参考第05章，正常运行不使用sudo。无需硬件的自测：两端将--port换为loop://，并加`--send '{"seq":1,"valid":true,"temperature_c":25}'`。在PowerShell中也使用单引号包裹JSON；这验证软件回环，不证明ESP32工作正常。

## 程序逐段理解

argparse定义命令行界面；serial_for_url既支持实体串口也支持回环。read设置0.5秒超时，所以Ctrl+C和计时退出不会无限等待。pending累积未完成的一行，收到LF再解析。若超过上限，进入discarding状态并丢弃直到下一次换行，防止坏数据不断占用内存。普通启动日志输出到stderr，有效JSON输出到stdout，便于后续管道处理。

CSV使用newline=''，由csv模块处理转义和跨平台换行；received_utc是电脑收到记录的时间。它包含设备发送和操作系统缓存延迟，不能直接作为精密采样时刻。finally关闭文件，Ctrl+C也能保留已写记录。write_timeout限制发送阻塞；断开设备时返回非零状态给自动化脚本。

## 验收、排错与练习

采集一分钟，CSV应有表头、递增序号和合理读数；拔掉传感器后应看到null而不是0；插入乱码不应终止记录。端口打开失败依次查占用、权限、线材和驱动。数据乱码先核对115200波特率及编码。

练习：统计丢帧。参考思路是按节点和启动周期分组后比较序号差，设备重启或计数器回绕必须重置基线，不把所有序号下降都判为数据丢失。来源：[pySerial API](https://pyserial.readthedocs.io/en/latest/pyserial_api.html)、[Python csv](https://docs.python.org/3/library/csv.html)。下一步：第55章。

> 验证状态以[验证报告](../../validation/REPORT.md)为准。命令中的端口与IP仅是示例，请按实际环境替换。
