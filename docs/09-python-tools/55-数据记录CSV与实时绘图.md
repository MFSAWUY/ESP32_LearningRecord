# 55 数据记录CSV与实时绘图

[返回总目录](../../README.md)

## 记录值、单位和测量时刻

前置：第10章误差、第54章采集。目标是把串口数据变成可分析曲线。完整程序：[plot_csv.py](../../examples/python/plot_csv.py)。它读取serial_tool生成的CSV，按received_utc计算相对时间，支持静态PNG和实时刷新。原始CSV是证据，图片只是展示；不要只保存截图。

温度均值为x̄=(Σxᵢ)/N，样本方差s²=Σ(xᵢ−x̄)²/(N−1)。只有近似独立、同分布测量才可用s/√N估计均值标准误。以1Hz连续采集缓慢变化温度时相邻样本相关，不能把1000条记录当成1000次独立实验。传感器准确度、重复性、分辨率也不是同一概念。

## 数据到曲线的过程

load_samples逐行读取时间和指定字段，第一条时间作为零点。无效测量转为NaN，Matplotlib会在曲线上形成断点；它不会用直线填补丢失区间。float转换保留数值类型，不能对字符串做字典序排序。日志中的raw_json保留额外字段，后续可以追溯节点状态。

Windows：
```powershell
.\.venv\Scripts\python.exe examples/python/plot_csv.py data/readings.csv --field temperature_c --output data/temperature.png
.\.venv\Scripts\python.exe examples/python/plot_csv.py data/readings.csv --field humidity_pct --watch
```
Linux：
```bash
.venv/bin/python examples/python/plot_csv.py data/readings.csv --field temperature_c --output data/temperature.png
.venv/bin/python examples/python/plot_csv.py data/readings.csv --field humidity_pct --watch
```
实时模式另开一个终端保持serial_tool采集。远程Ubuntu若没有图形桌面，使用--output生成文件后下载查看；不把GUI窗口能否弹出作为采集程序成败的标准。静态模式明确使用Agg后端，适合SSH和CI。

## 存储预算推导

若平均一行120字节，每2秒记录一次，每日大小为120×86400/2=5,184,000字节，约4.94MiB。CSV转义和额外字段会增加实际长度，应测量文件大小验证。4MB Flash除了应用还需分区表、NVS、OTA等，不能存储整天这样的日志。可用SD卡或电脑保存，并做滚动文件与保留期限。缓存减少Flash擦写次数，但增大掉电损失窗口；不要为了“实时”每10毫秒重写整个文件。

## 操作实验

先在室温静置采集两分钟，然后把手靠近传感器但不要触碰裸露引脚。观察响应延迟，再拔下传感器确认曲线断点。记录采样周期、环境和模块型号。若曲线完全平直，先检查是否画错字段、是否重复旧数据，而不是立即增加滤波。

练习：比较原始值和10点滑动平均。参考：移动平均的窗口时长约为N/fs，窗口越大噪声越小但响应越慢；比较两条曲线时保留时间轴并解释延迟。图标题必须区分真实测量与math_lab仿真。来源：[Matplotlib使用指南](https://matplotlib.org/stable/users/index.html)。下一步：第56章。

> 验证状态以[验证报告](../../validation/REPORT.md)为准。命令中的端口与IP仅是示例，请按实际环境替换。
