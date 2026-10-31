# 真机验收执行规程

2026-10-31，用户决定暂不连接外设。本规程是后续操作说明，所有物理实验保持待验收。本次没有烧录，也没有进行 OTA、功耗或 JTAG 实测。

## 先做只读盘点

PowerShell（手册根目录）：

```powershell
.\.tools\venv\Scripts\python.exe scripts/hardware_preflight.py --usb --output validation/preflight-windows.json
```

Ubuntu Bash（当前远端环境）：

```bash
cd /home/pc/wty/ESP32/handbook
/home/pc/wty/ESP32/test/desktop-venv/bin/python scripts/hardware_preflight.py --usb --output /home/pc/wty/ESP32/test/preflight-linux.json
```

脚本不打开串口、不触发复位、不烧录。退出0仅说明盘点完成，不能表示硬件实验通过。`--port 实际端口 --require-port`可以检查指定端口是否枚举到；没有端口时退出2。SSH不能把接在Windows上的USB设备自动转接给Ubuntu。

记录板卡丝印、芯片/Flash识别、供电方式、固件SHA256和实际端口。后续每次上传都先确认目标端口，按[硬件验收表](HARDWARE_CHECKLIST.md)保存串口原始日志和观察结果。

## 串口、OLED与传感器

1. 先运行[serial_info](../examples/arduino/serial_info/README.md)，确认经典ESP32和4MB Flash。USB枚举到CH340仅证明USB桥存在。
2. 断电后按[I²C扫描](../examples/arduino/i2c_scan/README.md)连接OLED：3V3→模块VCC、GND→GND、GPIO21→SDA、GPIO22→SCL；必须核对模块丝印，不按排针位置猜测。确认模块支持3.3V及其上拉电压。
3. 先扫描再运行[OLED示例](../examples/arduino/oled/README.md)。记录地址、SSD1306/SH1106、实际分辨率。地址0x3C本身不能证明控制器型号。
4. 观察边框、字符、刷新和菜单。串口“初始化成功”不能证明屏幕显示正确。
5. 有SHT30后按[传感器示例](../examples/arduino/sht30/README.md)运行，核对CRC、合理温湿度和有效标记。断电断开传感器再重新上电，应报告无设备；断电恢复接线再启动，记录恢复结果。需要热插拔测试时，先确认模块及总线电气条件允许，不默认带电拔杜邦线。

两线设备共用GPIO21/22时，需保证地址不同并检查并联上拉，不能为每个模块单独随意配置一组同名总线。

## 网络恢复和持久化

Wi-Fi配置只写`config.local.h`，不将密码加入提交。按[网络时钟](../examples/projects/oled_clock/README.md)和[环境监测](../examples/projects/environment_station/README.md)运行，分别记录正常联网、关闭AP、恢复AP、重新获取时间的时间点。计时起点和成功判据应明确，不只记录“最后连上”。

用[NVS示例](../examples/esp-idf/nvs_storage/README.md)记录首次值、写入后值、按EN复位值和断电再上电值。擦除Flash会影响配置，应另作受控实验；普通重启不能用擦除替代。

## OTA正常升级和失败回滚

完整代码与参数说明见[OTA工程](../examples/esp-idf/ota/README.md)。先串口烧录匹配的bootloader、分区表和正常基线应用，确认双OTA槽、回滚启用、真实HTTPS证书与时间同步。公开`example.invalid`编译参数不能用于上板联网。

| 测试 | 操作与必须保存的证据 |
|---|---|
| 正常升级 | 基线版本1.0.0；服务器提供不同版本的正常应用镜像；记录下载、重启、运行分区和`new image confirmed` |
| 相同版本 | 提供与运行版本相同的镜像；应跳过，无新镜像确认记录 |
| 证书或主机名不匹配 | 使用受控错误服务器配置；TLS拒绝且旧应用仍能启动 |
| 下载中断 | 下载途中关闭测试服务器或断开网络；记录错误并确认旧启动镜像仍可用 |
| 自检失败回滚 | 已确认的正常基线存在后，通过OTA写入不同版本、启用`EXAMPLE_FORCE_SELFTEST_FAIL`的新镜像；应出现拒绝自检、重启和返回旧分区的证据 |
| 断电恢复 | 确认基线可恢复后，在记录的升级阶段断电再恢复，保存启动版本及分区；不要把单次通过推断为所有断电时刻都通过 |

故意失败的镜像不能作为唯一初始固件。回滚后服务器应停止提供失败镜像，避免反复升级。编译self-test-fail分支只证明代码可编译，不证明bootloader实际回滚。测试不要求写eFuse或开启不可逆安全配置。

## 功耗测量

运行[深睡工程](../examples/esp-idf/deep_sleep/README.md)或[电池节点](../examples/projects/battery_node/README.md)，记录电源电压、测量位置、仪表量程/带宽/采样率、是否连接USB、OLED、传感器、指示灯及稳压器。

电流表串联在供电路径，不能像测电压一样跨接电源。USB端整板电流包括CH340和板载器件；3.3V支路测量不是整板USB输入功耗。避免USB与外部电源未经确认同时供电。无适合的电流仪表时保持待测。

对实测各阶段电流I、时间t，使用`I平均=Σ(I×t)/Σt`；电压不同或包含转换器时按能量`E=Σ(V×I×t)`计算并计入效率。记录至少多个完整唤醒周期，分别给出活动峰值、睡眠稳态和平均值，说明仪表是否漏掉短脉冲。模型仿真不能替代这些读数。

## 经典ESP32的JTAG

CH340是串口桥，不能用其替代外置JTAG探针。经典ESP32通常使用GPIO12/MTDI、GPIO13/MTCK、GPIO14/MTMS、GPIO15/MTDO及公共地；GPIO12涉及启动配置，探针电平及板卡上拉必须按官方接线说明核对。探针、驱动和OpenOCD配置取决于真实型号，不能在型号未知时套用其他板卡配置。

准备与被烧录固件完全一致的ELF，参考[第51章](../docs/08-reliability/51-日志崩溃分析与JTAG调试.md)。验收必须包括：OpenOCD识别目标、GDB连接、在`app_main`或任务函数命中断点、读取变量、查看调用栈、单步、继续和复位后再次命中。保存OpenOCD/GDB日志、探针型号、固件SHA256及ELF路径。

生成ROM gdbinit文件仅证明构建调试辅助文件可用；没有探针与目标连接，不能声明JTAG已验收。官方参考：[ESP32 JTAG调试](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/jtag-debugging/index.html)。
