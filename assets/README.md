# 数学曲线与接线说明

这里的PNG和CSV均由 `examples/python/math_lab.py` 生成，是明确标注的模型仿真。无真实硬件测量数据。

- rc：3.3V阶跃，R=10kΩ，C=1µF，τ=10ms，初始电压0。
- filter：100Hz采样，阶跃叠加18Hz正弦干扰，τ=0.1s一阶低通。
- alias：70Hz正弦以100Hz采样，与−30Hz正弦在采样点不可区分。
- pid：dt=10ms，时间常数0.5s的一阶对象，输出限制0–1，目标0.7，演示条件积分和测量微分。

重新生成示例（两端均用对应虚拟环境解释器）：
```text
python examples/python/math_lab.py rc --output assets/rc.csv --plot assets/rc.png
python examples/python/math_lab.py filter --output assets/filter.csv --plot assets/filter.png
python examples/python/math_lab.py alias --output assets/alias.csv --plot assets/alias.png
python examples/python/math_lab.py pid --output assets/pid.csv --plot assets/pid.png
```

接线图优先用章节中的引脚表，系统流程使用Mermaid，以避免把未知板卡的排针位置画成已确认布局。
