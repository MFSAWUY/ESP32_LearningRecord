# ble

经典 ESP32、4 MB Flash，Arduino-ESP32 3.3.12 / PlatformIO espressif32 8.0.0。
源码为本手册原创教学实现；依赖库保留各自许可证。

## 构建与运行

先按手册第04/05章配置对应操作系统。Windows PowerShell 和 Linux Bash 在本目录均执行：

```text
pio run
pio device list
pio run -t upload --upload-port YOUR_PORT
pio device monitor --port YOUR_PORT --baud 115200
```

将 YOUR_PORT 替换成枚举得到的端口。使用 VS Code PlatformIO 终端；在没有全局 pio 时不要重复安装另一套工具链。先关闭监视器再烧录。
若 include 中有 config.example.h，首次可将其复制为 config.local.h；修改为自己的引脚/地址/凭据。模板不含真实密码，凭据不提交。

## 接线、原理与验收

请从[手册总目录](../../../README.md)找到同名案例对应章节；接线以 GPIO 编号及模块丝印为准。
固件在 ESP32 上执行，电脑负责构建烧录。运行前断电接线，GPIO 为 3.3 V 逻辑。

## 验证状态

源码已作人工静态审查。编译、Windows/Linux 与真机结果以[验证目录](../../../validation/)记录为准；本说明不声称已上板测试。
