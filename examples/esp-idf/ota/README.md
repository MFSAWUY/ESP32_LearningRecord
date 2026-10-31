# HTTPS OTA 与失败回滚

经典 ESP32 / 4 MB Flash / ESP-IDF 6.1，无PSRAM依赖。对应手册第49、53章。

## 接线、参数与代码讲解

无需额外外设，但需要可访问的Wi-Fi、SNTP服务及HTTPS应用镜像服务器。默认SSID和URL为空，启动只打印提示并退出升级流程；空配置也可正常编译。

先运行`idf.py menuconfig`，在`Handbook OTA`填写SSID、密码、HTTPS URL和SNTP服务器。真实参数保存在本地sdkconfig，不提交。默认使用IDF证书包；自建CA时将`main/server_root.example.h`复制为`main/server_root.h`，把完整真实CA证书填入SERVER_ROOT_PEM。服务器证书的主机名与URL必须匹配，证书有效期需要正确时间。

两个应用槽各0x1E0000字节（1,966,080），完整分区终点0x400000；源码的partitions.csv和sdkconfig.defaults已启用双槽与回滚。第一次必须串口烧录匹配的引导程序、分区和应用。OTA URL提供`build/ota.bin`应用镜像，不提供全片merged镜像。

根CMake中的PROJECT_VER初始为1.0.0。保留该基线后改为1.0.1并构建新镜像，放到HTTPS服务器；旧固件运行后应下载、验证、重启，新固件自检成功打印`new image confirmed`。相同版本会跳过升级。仅版本不等不代表支持安全的版本顺序管理。

测试回滚时必须先有已确认的正常旧应用，再为新镜像启用`EXAMPLE_FORCE_SELFTEST_FAIL`并改变版本，经OTA写入。新镜像应拒绝自检并回到旧槽。不要把故意失败的镜像当唯一初始应用烧入后期待存在可回滚版本。若旧固件URL一直指向失败镜像可能重复尝试，测试后恢复服务器正常镜像。

代码将Wi-Fi连接等待限制30秒、SNTP等待30秒、下载会话180秒；错误会打印并保留旧启动镜像。初始化失败采用ESP_ERROR_CHECK；示例不在NVS异常时自动擦除用户数据。教程未启用硬件Secure Boot或烧写eFuse。

练习：错误CA、域名不匹配、网络中断、相同版本、过大镜像、自检失败分别应在哪个阶段被拒绝？记录旧版本、目标版本、分区、错误和重启原因，而不是只记录“能升级”。

## 从打开工程到运行

1. 在VS Code或终端打开本工程目录，先按第04/05章激活ESP-IDF 6.1环境。
2. 按[Windows PowerShell与Linux Bash完整命令](../README.md)首次设置esp32目标、构建，再枚举串口并输入实际端口。构建本身不要求连接开发板。
3. 按本页接线，烧录后以115200波特率观察日志；关闭其他占用同一串口的软件。
4. 核对上述预期现象与错误路径，并填写真机验收表。预期日志是教学说明，不是真机运行证据。

源码入口为[main/main.c](main/main.c)，组件依赖见[main/CMakeLists.txt](main/CMakeLists.txt)，默认目标和Flash参数见[sdkconfig.defaults](sdkconfig.defaults)。根CMake启用MINIMAL_BUILD，只构建main及声明的依赖；依赖缺失应修正组件清单，不靠无关组件偶然传递头文件。

## 来源与验证

教学代码为本手册原创，采用[MIT](../../../LICENSE)；ESP-IDF及其组件保持各自许可证。API参考[ESP-IDF v6.1官方文档](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/)。实际编译状态见[验证报告](../../../validation/REPORT.md)，物理实验填[硬件验收表](../../../validation/HARDWARE_CHECKLIST.md)。
