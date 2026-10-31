# 40 Arduino到ESP-IDF的迁移

[总目录](../../README.md) · 目标：ESP32-D0WD / 4 MB Flash / ESP-IDF 6.1。前置：第18–39章相关Arduino实验。下一步：按总目录进入后一章。


## 学习目标和迁移对象

目标是把“按钮每按一次，LED切换一次”的Arduino实验迁移为原生IDF工程。电路、去抖和状态机逻辑继续适用；需要改变的是启动入口、驱动配置、时间单位和错误处理。Arduino提供setup/loop，IDF提供app_main；两者底层都有系统初始化，不能把app_main误认为CPU复位后的第一条指令。

app_main可以创建任务后返回，本例为了清楚展示轮询，将循环留在入口中。若源文件改用C++，入口需要C链接形式：`extern "C" void app_main()`，否则链接名称可能不符合启动代码的期待。

## 接线与接口对应

GPIO25接330Ω电阻，再接LED正极，LED负极接GND；GPIO27接按键，另一端接GND。输入启用内部上拉，松开为1，按下为0。外接LED避免依赖未知开发板上的电源指示灯；不要拿只有输入能力的引脚驱动LED。

| Arduino | IDF | 迁移要点 |
|---|---|---|
| pinMode | gpio_config | 方向、上拉和掩码分别设置 |
| digitalRead | gpio_get_level | 输入要有确定默认电平 |
| digitalWrite | gpio_set_level | 逻辑有效高低由电路决定 |
| delay | vTaskDelay | 参数是tick，毫秒先转换 |
| Serial.printf | ESP_LOGI | 等级、标签、格式符都要正确 |

源码用`1ULL << pin`产生64位掩码。32位常量无法安全表示所有ESP32引脚位，这是位宽知识在驱动配置中的直接应用。配置结构未指定字段按C规则置零，输出与输入分开初始化便于检查电气状态。

## 去抖状态机与时间推导

程序保存previous、stable、changed：分别表示最近原始电平、已接受的稳定电平和最近变化时刻。原始输入每次改变就重新计时；持续30ms未改变才更新stable。只有stable变为0时翻转LED，所以持续按住不重复触发。

```c
if (raw != previous) {
    previous = raw;
    changed = now;
}
if (raw != stable && now - changed >= pdMS_TO_TICKS(30)) {
    stable = raw;
    if (stable == 0) {
        led = !led;
        gpio_set_level(LED, led);
    }
}
```

轮询周期为5ms，真实边沿发生在两次采样之间时，检测延迟约0～5ms；确认稳定也受离散采样影响。因此30ms是判定阈值，并非严格总响应时间。无符号时间差可处理一次正常回绕，但比较窗口应远小于计数器周期。

## 错误处理与调试顺序

初始化参数非法会使本例无法正确运行，因此用ESP_ERROR_CHECK尽早暴露；业务循环中的正常按钮状态不是错误。后续传感器超时、网络掉线等属于可恢复情况，不能一律断言重启。先学会区分错误性质，再选择停止、重试或降级。

先只接LED确认极性与限流，再接按键。输入随机变化先查上拉和共地；LED不亮先查极性和GPIO电平，不要删除电阻尝试。编译错误查看API版本和组件依赖；串口打不开则检查占用和端口，不应混为同一类故障。

## 验收与练习

短按10次应切换10次，按住2秒只切换一次，松开不切换。串口预期格式为pressed及LED状态，时间戳与真实运行有关。实际验收应记录次数和是否误触发。

练习：增加长按。参考：稳定按下时保存时间，达到阈值后设置一次性标志，松开时清除；若长短按互斥，可以在松开时根据持续时间只发布一个事件。

## 来源

- [GPIO官方接口](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/gpio.html)
- [Arduino迁移指南](https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html)

## 完整工程与跨平台运行

[完整源码](../../examples/esp-idf/gpio/main/main.c) · [构建文件](../../examples/esp-idf/gpio/CMakeLists.txt)。先按第04或05章激活IDF环境，在手册根目录执行。

Windows PowerShell：

```powershell
Set-Location examples/esp-idf/gpio
idf.py set-target esp32
idf.py build
$port = Read-Host '输入实际串口'
idf.py -p $port flash monitor
```

Linux Bash：

```bash
cd examples/esp-idf/gpio
idf.py set-target esp32
idf.py build
read -r -p '输入实际串口: ' port
idf.py -p "$port" flash monitor
```

set-target仅首次配置或切换芯片时需要，已有自定义配置先备份。退出监视器用Ctrl+]；烧录前关闭其他占用串口的程序。仅编译无需硬件，物理行为需真机验证。[验证记录](../../validation/)区分静态检查、构建和实测，不以预期日志冒充实测。

资料基线：2026-10-29；示例是原创教学代码，遵循仓库许可证。
