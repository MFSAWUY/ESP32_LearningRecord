"""Regenerate handbook navigation from the actual chapter and project files."""
from pathlib import Path

root=Path(__file__).resolve().parents[1]
groups={"01-guide":"认识设备与准备环境", "02-math-and-physics":"数学与物理基础", "03-c-and-cpp":"C/C++详细复习", "04-peripherals":"Arduino与基础外设", "05-sensing-and-control":"传感器与控制", "06-networking":"无线通信与物联网", "07-esp-idf-and-rtos":"ESP-IDF与实时系统", "08-reliability":"存储功耗与工程质量", "09-python-tools":"Python上位机", "10-projects":"完整综合项目", "11-advanced":"进阶应用与拓展", "12-reference":"查阅与复习"}
text='''# ESP32 系统学习手册

面向有少量 C/C++ 和电路基础、希望系统复习并独立完成项目的学习者。以经典 ESP32-D0WD、4 MB Flash、CH340 Type-C 开发板及四针 I²C OLED 为主线。

**学习路线：基础复习 → VS Code + PlatformIO + Arduino → ESP-IDF + FreeRTOS → 完整项目。** 数学推导、物理模型、代码、接线和验收放在同一学习链路中。

## 从哪里开始

1. 阅读01–05章，确认板卡、配件和电脑环境。OLED型号未确认时先扫描地址，再确认控制器及分辨率。
2. 用18章完成串口与芯片信息，用24–25章点亮现有OLED，再完成58章网络时钟。你目前已有的配件足够进行这条入门路线。
3. 按06–17章复习基础，再顺序学习外设和网络。每章练习有参考思路，重点理解代码改变后为什么出现相应现象。
4. 用40–53章补齐系统工程，最后完成59–62章综合项目。需要额外硬件的章节会注明。

**电脑运行与开发板运行：** Python上位机及仿真脚本直接运行在Windows/Linux；Arduino和ESP-IDF固件由电脑交叉编译，再在ESP32上运行。软件回环、仿真和编译成功均不能替代真机实验。

## 完整目录

'''
for directory,title in groups.items():
    text+=f'### {title}\n\n'
    for path in sorted((root/'docs'/directory).glob('*.md')):
        text+=f'- [{path.stem.replace("-", " ",1)}]({path.relative_to(root).as_posix()})\n'
    text+='\n'
text+='''## 示例工程与工具

每个固件工程可以独立打开。先阅读工程README和章节，再修改本地配置并构建；示例端口、IP和密码占位符必须按实际环境替换。工程引脚规划不等于任意开发板排针位置。

| 类型 | 工程入口 |
|---|---|
'''
for category,label in [('arduino','Arduino / PlatformIO'),('esp-idf','原生 ESP-IDF'),('projects','综合项目')]:
    links=[]
    for directory in sorted((root/'examples'/category).iterdir()):
        manifest = 'CMakeLists.txt' if category == 'esp-idf' else 'platformio.ini'
        if directory.is_dir() and (directory / manifest).is_file():
            links.append(f'[{directory.name}](examples/{category}/{directory.name}/)')
    text+=f'| {label} | '+', '.join(links)+' |\n'
text+='''| Python | [安装与命令](examples/python/README.md) · [串口](examples/python/serial_tool.py) · [绘图](examples/python/plot_csv.py) · [网络](examples/python/network_tool.py) · [控制面板](examples/python/dashboard.py) · [数学实验](examples/python/math_lab.py) |

## 原理图与数学实验

章节使用接线表和Mermaid图说明电路、数据流和状态。以下曲线由随附Python程序生成，**全部为模型仿真，不是真实传感器测量**。

| RC阶跃响应 | 一阶低通滤波 |
|---|---|
| ![RC模型](assets/rc.png) | ![滤波模型](assets/filter.png) |

[混叠演示](assets/alias.png) · [离散PID模型](assets/pid.png) · [生成参数与说明](assets/README.md)

## 版本、验证和使用边界

主线固定 Espressif32 8.0.0 / Arduino-ESP32 3.3.12；独立 ESP-IDF 6.1。Python直接依赖固定在requirements.txt。Windows 11和Ubuntu 24.04为文档目标；实际验证机器与结果分别记录，远程Ubuntu实际为22.04.5。

- [验证报告](validation/REPORT.md)：实际构建、Python运行、未验证项和限制。
- [来源与许可索引](docs/12-reference/72-官方资料代码来源与许可证.md)：官方资料、第三方库和版本追溯。
- [结构检查](scripts/check_handbook.py)：72章数量、本地链接、代码围栏和Python语法。
- [批量构建](scripts/build_examples.py)：只构建，不上传固件。
- [硬件验收表](validation/HARDWARE_CHECKLIST.md)：需要开发板和外设完成的实验记录。

每个关键推导说明假设、单位、数值例题和误差来源。外接负载只使用安全低压实验，GPIO采用3.3V逻辑。复杂摄像头、USB或无线协议的能力按目标芯片区分，不默认你的经典ESP32具备整个家族的所有功能。
'''
(root/'README.md').write_text(text,encoding='utf-8')
