# 41 CMake组件Kconfig与工程配置

[总目录](../../README.md) · 目标：ESP32-D0WD / 4 MB Flash / ESP-IDF 6.1。前置：第18–39章相关Arduino实验。下一步：按总目录进入后一章。


## 学习目标：理解构建链

固件除了业务源码，还包含启动代码、系统配置、驱动、链接脚本和分区表。CMake描述文件与依赖；Ninja安排增量构建；交叉编译器生成ESP32指令；链接器解析符号并安排内存；idf.py组织配置、构建和烧录。电脑CPU的可执行文件不能直接作为ESP32固件。

构建前运行`idf.py --version`，确认激活的是手册指定环境。IDE是交互入口，底层工具链仍需正确配置；在普通终端找不到idf.py不能说明源代码有问题。

## 两层CMake文件

根目录先引入IDF项目规则，再声明工程：

```cmake
cmake_minimum_required(VERSION 3.22)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)
project(gpio)
```

`MINIMAL_BUILD ON`仅纳入main及其依赖，减少无关组件的构建时间。它沿用IDF 6.1官方hello_world示例的方式，不能省略真正使用组件的REQUIRES声明。

main组件注册源码和依赖：

```cmake
idf_component_register(SRCS "main.c"
    INCLUDE_DIRS "." REQUIRES esp_driver_gpio)
```

头文件只提供声明，依赖关系负责把实现与所需接口纳入构建。REQUIRES会传播公开依赖；PRIV_REQUIRES适合实现内部使用的依赖。初学阶段明确列出依赖，后续按组件公开头文件是否暴露相关类型整理边界。

如果include大小写与文件名不同，Windows可能掩盖问题，Linux则可能直接失败。因此文件名和引用要完全一致，路径尽量无空格且使用清楚的相对关系。构建目录中的绝对缓存路径不能作为跨平台源码一起搬运。

## sdkconfig、defaults和Kconfig

sdkconfig.defaults保存推荐初始值；sdkconfig保存当前工程配置；构建生成的sdkconfig.h供C/C++读取宏。修改defaults不一定覆盖已经存在的sdkconfig，实际行为必须按当前配置判断。menuconfig是检查和修改配置的入口。

本书默认esp32与4MB Flash。配置更大的Flash不会增加物理容量，分区超出实际地址会导致错误。应用可通过Kconfig.projbuild增加SSID、服务器地址等参数。把密码编译进固件仍可能被提取，不能把配置字符串当作安全密钥存储，也不要提交带真实凭据的sdkconfig。

初次set-target后通常直接build。更换芯片、配置结构变化或缓存异常时，先保存必要配置再fullclean，不要反复删除整个工具链。清除构建缓存和清除设备Flash是两种完全不同的操作。

## 编译、链接与产物

编译错误常来自语法、类型或缺少头文件；链接错误如undefined reference常来自缺少实现、依赖或C/C++名称修饰；运行错误则要查日志和硬件。按阶段分类能避免用安装驱动的方法处理链接错误。

ELF保存符号，适合回溯解码；应用bin用于应用分区与OTA；bootloader和partition-table分别负责引导和布局。完整串口烧录可能写多项产物，不能把其中任意一个bin都称为应用固件。回溯解析必须使用与故障镜像匹配的ELF。

构建报告的静态RAM占用不包含所有运行峰值。任务栈、堆、网络缓冲和DMA分配仍需动态测量；Flash空间充裕也不代表SRAM足够。

## 配置实验、验收和练习

先构建GPIO原始工程并保存摘要；将日志等级Info改为Warning，重建后普通信息日志应消失，恢复设置后重新出现。观察源码未变而固件变化，理解配置参与构建的作用。

练习：把LED引脚作为Kconfig整数参数。参考：范围0～39仍包含不可输出、Flash占用或启动敏感引脚，数值校验不能代替硬件能力和接线校验。工程默认继续用GPIO25作为已明确的实验引脚。

## 来源

- [构建系统](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/build-system.html)
- [项目配置](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/kconfig-reference.html)

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
