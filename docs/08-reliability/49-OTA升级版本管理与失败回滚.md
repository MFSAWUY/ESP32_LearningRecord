# 49 OTA升级版本管理与失败回滚

[返回总目录](../../README.md)

串口先枚举再选择；本章端口变量的初始化见[Windows/Linux公共命令](../../examples/esp-idf/README.md)。

## 升级事务与分区

前置：第34、47、53章。完整工程：[ota](../../examples/esp-idf/ota/)。OTA不是把网络文件覆盖当前运行区域，而是下载到另一应用槽，验证后改变下一次启动选择，启动自检成功才确认新版本。网络失败时旧固件仍可启动，回滚解决新固件无法正常运行的问题，但不是所有掉电故障的万能保证。

本例4MB分区：NVS 0x9000/0x4000，otadata 0xD000/0x2000，phy 0xF000/0x1000；ota_0从0x10000占0x1E0000，ota_1从0x1F0000占0x1E0000，storage从0x3D0000占0x30000，终点0x400000恰好4MiB。每个应用槽约1.875MiB，实际固件必须小于槽大小，不能只检查整片Flash总量。

## 完整代码的四个阶段

第一，启动时查询正在运行分区的状态。如果处于PENDING_VERIFY，则运行self_test；失败调用mark_app_invalid_rollback_and_reboot，成功调用mark_app_valid_cancel_rollback。示例自检检查堆完整性和可用内存，真实产品还应检查关键传感器、存储与业务功能。

第二，检查SSID和HTTPS URL配置；空值只打印提示，不发起更新。Wi-Fi连接最多等待30秒；SNTP同步用于正确验证证书有效期。HTTPS必须校验可信根和主机名，不能通过跳过校验来“修好”连接。

第三，begin后读取镜像描述，拒绝头错误或相同版本；perform逐步下载，设置最长180秒会话。失败调用abort并保持旧启动镜像。第四，finish确认完整性后重启。版本字符串不相等仅用于教学，生产升级还需要明确版本顺序、硬件兼容、签名和防降级策略。

## 实验操作

在工程目录先用idf.py menuconfig配置Wi-Fi、HTTPS URL和SNTP服务器；自建证书则按README创建server_root.h并嵌入真实CA。默认空证书使用证书包，不能随意填假PEM。Windows用idf.py -p $serialPort flash monitor，Linux对应 `idf.py -p "$serial_port" flash monitor`；初始分区及引导程序通过串口烧录，随后才测试OTA。

准备两个不同PROJECT_VER的镜像，HTTPS服务器返回应用bin，而不是merged全片镜像。先升级到正常版本，再构建启用CONFIG_EXAMPLE_FORCE_SELFTEST_FAIL的版本验证回滚。每次记录原版本、新版本、槽位和重启原因；确认仍能串口恢复后再进行故障注入。

## 验收与练习

测试错误CA、域名不匹配、网络中断、过大镜像、版本不变、自检失败。拒绝更新的原因应可观察，旧固件应保持可用。回滚必须预先有可用旧应用和正确引导配置，不保证对任意损坏布局都有效。练习：设计兼容多个硬件版本的manifest，字段应包括目标芯片、硬件范围、版本、镜像摘要及签名；摘要校验与签名认证各自解决不同问题。来源：[IDF OTA](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/ota.html)。下一步：第50章。
