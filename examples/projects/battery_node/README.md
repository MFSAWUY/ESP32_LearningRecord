# battery_node

本工程目标：经典 ESP32、4 MB Flash、不依赖 PSRAM。源码属于手册原创教学代码；框架/第三方库保留各自许可证。

## 配置与运行

在本目录打开终端。`include/config.example.h` 默认可以编译；涉及网络时复制为 `include/config.local.h` 并填入参数。不要上传真实凭据。

Windows PowerShell：
```powershell
Copy-Item include/config.example.h include/config.local.h
pio run
pio device list
$serialPort = Read-Host '输入枚举出的实际串口'
pio run --target upload --upload-port $serialPort
pio device monitor --port $serialPort --baud 115200
```

Linux Bash：
```bash
cp include/config.example.h include/config.local.h
pio run
pio device list
read -r -p '输入枚举出的实际串口: ' serial_port
pio run --target upload --upload-port "$serial_port"
pio device monitor --port "$serial_port" --baud 115200
```

端口通过枚举后输入，不能依据其他设备的端口号猜测。不要同时打开两个串口监视器。复制配置仅在首次进行，避免覆盖自己的修改。

## 教程与验证

详细接线、模块解释、实验验收见[总目录](../../../README.md)的综合项目章节。
本文件不表示构建或真机已通过；准确记录见[验证报告](../../../validation/REPORT.md)。
