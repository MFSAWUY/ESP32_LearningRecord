# 42 ESP-IDF外设驱动与事件机制

[总目录](../../README.md) · 目标：ESP32-D0WD / 4 MB Flash / ESP-IDF 6.1。前置：第18–39章相关Arduino实验。下一步：按总目录进入后一章。


## 学习目标与接线

通过SHT30理解总线句柄、设备句柄、事务与应用事件。模块VCC接3V3、GND接GND、SDA接GPIO21、SCL接GPIO22；默认地址0x44，若模块是0x45要修改配置。先确认模块3.3V兼容性和上拉电阻，再通电。

OLED与传感器可共享总线，但地址不得冲突，并联上拉后的等效阻值要检查。先用短线和100kHz，不把内部上拉视为任何线长下都足够的保证。

## 资源模型与生命周期

i2c_new_master_bus配置控制器、引脚、时钟源和毛刺过滤；i2c_master_bus_add_device配置地址与速度。一条物理总线可对应多个设备句柄，不需要每增加一个地址就重新建立控制器。新旧I²C驱动不能在同一控制器随意并用。

动态关闭设备时，先停止新事务、等在途操作完成，再移除设备，最后删除总线。句柄删除后其他任务仍然使用它会产生资源生命周期错误。常驻示例可以在启动后一直持有句柄，但应知道其所有者是谁。

## 命令、CRC与换算

程序发送0x24、0x00，选择高重复性、无时钟拉伸的单次测量，等待20ms后读6字节。数据顺序是温度高低字节和CRC，再是湿度高低字节和CRC。CRC初值0xFF、多项式0x31；两组都通过后才发布读数。

```c
uint16_t t = ((uint16_t)data[0] << 8) | data[1];
uint16_t h = ((uint16_t)data[3] << 8) | data[4];
```

传感器规定的映射为：

$$T=-45+175S_T/65535,qquad RH=100S_H/65535.$$

温度单位℃，相对湿度单位%RH。原始温度码30000对应约35.11℃。这是SHT30自己的编码公式，不能当作任何ADC都通用的电压公式。映射范围也不等于全范围精度相同，应结合测量误差和工作条件。

CRC检测传输错误，不认证设备身份，也不修复坏数据。CRC错误时丢弃本次样本；若展示上次有效读数，要同时显示年龄，不能假装它是新测量。

## 超时、恢复与事件

每次I²C发送/接收设100ms上限；失败后记录原因并在下一采样周期重试，不无限忙等。初始化参数错属于不能继续工作的条件，运行时拔掉传感器则属于可恢复失败，主程序仍应保持日志和其他功能。

I²C API超时参数与FreeRTOS等待的单位可能不同。本例驱动参数以毫秒解释，任务延迟用pdMS_TO_TICKS转换。不能看见数字100就假定每个函数都等待100ms。

工程扩大后，将时间戳、数值、有效标志和错误码组成消息发送给显示或网络任务。系统事件适合状态变化，高频大块样本适合专用队列或缓冲。回调中不要执行长时间网络请求、文件写入和屏幕刷新，把工作交给任务。

单次API线程安全不保证“发命令—等待—读结果”整个序列原子。多个任务访问同一传感器时应加事务互斥，或让一个任务作为唯一所有者，其他模块通过消息请求。

## 实验和参考练习

先运行地址扫描，再进行测量。传感器缺失时预期是错误日志，不是零温度或伪造读数。拔除后重新接入应能继续采样；用手靠近时数值缓慢变化。剧烈跳动先查供电、线长和CRC，不应先用滤波掩盖坏连接。

练习：连续三次失败才发离线事件，恢复后只发一次上线事件。参考：同时维护计数和状态，只有状态边沿变化才广播，避免每个轮询周期刷屏。

## 来源

- [I²C驱动](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/i2c.html)
- [事件循环](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_event.html)
- [SHT30官方资料](https://sensirion.com/products/catalog/SHT30-DIS-B)

## 完整工程与跨平台运行

[完整源码](../../examples/esp-idf/i2c_sensor/main/main.c) · [构建文件](../../examples/esp-idf/i2c_sensor/CMakeLists.txt)。先按第04或05章激活IDF环境，在手册根目录执行。

Windows PowerShell：

```powershell
Set-Location examples/esp-idf/i2c_sensor
idf.py set-target esp32
idf.py build
$port = Read-Host '输入实际串口'
idf.py -p $port flash monitor
```

Linux Bash：

```bash
cd examples/esp-idf/i2c_sensor
idf.py set-target esp32
idf.py build
read -r -p '输入实际串口: ' port
idf.py -p "$port" flash monitor
```

set-target仅首次配置或切换芯片时需要，已有自定义配置先备份。退出监视器用Ctrl+]；烧录前关闭其他占用串口的程序。仅编译无需硬件，物理行为需真机验证。[验证记录](../../validation/)区分静态检查、构建和实测，不以预期日志冒充实测。

资料基线：2026-10-29；示例是原创教学代码，遵循仓库许可证。
