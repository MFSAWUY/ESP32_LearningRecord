# 2026-10-31 缓存修复与两端编译完成记录

## 当前结果

- Windows：真实D盘目录下11个原生IDF默认工程、联网OTA、自检失败OTA均通过，13个镜像容量检查通过；13个新CMake缓存无已撤销映射引用，ROM符号文件正常生成。P盘映射和D:\ehb均未恢复。
- Ubuntu22.04.5：Arduino/综合工程28/28通过，29个实际镜像通过4MiB分区检查；原生IDF11/11、联网OTA和自检失败OTA通过，13个镜像容量检查通过。工具链及Python依赖安装完成。
- Windows/Ubuntu：Python各9项、桌面C++各1项通过。Windows编译器MSVC，Ubuntu GNU，均为实际执行。
- Git：本地main已初始化，使用已有身份建立有效HEAD，未配置remote、未上传GitHub。Actions工作流已补齐并做本地解析检查，未在GitHub执行。
- 用户暂不接外设，因此真机OLED、SHT30、网络恢复、NVS重启、OTA回滚、功耗和完整JTAG继续标记待验收。没有烧录。

## 旧D:\ehb是否需要修复

需要处理继续构建所用的缓存。CMake缓存和Ninja规则包含绝对路径，旧目录撤销后不宜直接沿用，也不应简单全局替换路径。此次保留旧缓存作为历史证据，在真实工作区新建独立源码副本与构建目录，重新配置、编译和检查。

新入口是[scripts/windows_idf.ps1](../scripts/windows_idf.ps1)。旧缓存中仍可检索到D:\ehb，这是有意保留的历史数据，不代表新构建依赖该路径；新缓存检查见[cache-check.json](windows-repair/cache-check.json)。

## 缓存与日志准确位置

Windows基准根目录：`D:\VSCode_data\C++\ESP32\handbook`。下表的相对目录均在该根目录下。

| 位置 | 内容 |
|---|---|
| `.tools/platformio/packages` | Windows已安装框架、编译器、CMake、Ninja、ROM ELF等 |
| `.tools/platformio/.cache/downloads` | Windows下载缓存与历史残留临时文件 |
| `.tools/platformio/penv/.espidf-6.1.0` | Windows IDF专用Python环境 |
| `.tools/venv` | Windows桌面Python与PlatformIO环境 |
| `.tools/platformio/native-suite`、`native-build`、`ota-configured`、`idf-short`、`idf-validation` | 历史IDF源码副本/缓存，部分仍含旧路径 |
| `.tools/idf-current/default` | 当前11个默认IDF工程，sources/是副本，builds/是新缓存与镜像 |
| `.tools/idf-current/ota-configured`、`ota-selftest-fail` | 当前两种OTA配置缓存与镜像 |
| `validation/windows-repair` | 本轮Windows成功记录、容量和缓存检查 |
| `validation/windows-repair/before-first-commit` | Git空仓库导致的10项失败和当时日志快照 |
| `validation/linux-resume` | 从Ubuntu复制回来的本轮成功/失败记录 |
| `validation/logs`、各历史JSON | 早期构建、下载及外链记录 |
| `.tools/sync-*.log` | 历次源码同步输出，不放入源码包 |

Ubuntu基准测试目录：`/home/pc/wty/ESP32/test`；源码仍为`/home/pc/wty/ESP32/handbook`。

| Ubuntu位置 | 内容 |
|---|---|
| `test/platformio/packages` | Arduino框架、GCC14.2及烧录工具 |
| `test/platformio/.cache/downloads` | Arduino下载缓存，旧中断文件仍保留 |
| `test/builds` | 28个Arduino/综合项目的build和libdeps |
| `test/desktop-venv` | 桌面Python、PlatformIO环境 |
| `test/idf-platformio/packages` | 独立IDF6.1、GCC15.2、ROM ELF，避免切换Arduino包 |
| `test/idf-platformio/.cache/downloads` | 本轮原生IDF下载缓存 |
| `test/idf-venv` | 原生IDF独立Python环境 |
| `test/idf-current/default`、`ota`、`ota-fail` | 原生IDF源码副本、缓存和固件 |
| `test/resume-20261031` | 本轮全部主要安装、构建、容量、Python及盘点记录 |
| `test/arduino-before-u8g2-retry-20261031` | 首轮26通过、2项U8g2超时的完整快照 |
| `test/python-venv` | 最初失败的小环境残留；不是当前可用环境 |

缓存和工具文件没有随Git或源码ZIP分发。历史失败文件没有作为清理步骤删除；未来若清理，先归档记录，避免把已安装包与可再生构建缓存混为一谈。

## 本轮失败到成功的记录

| 事件 | 修复与最后结果 | 证据 |
|---|---|---|
| 旧CMake包含D:\ehb | 真实目录全新配置，11默认+2OTA通过；旧缓存保留 | [Windows新记录](windows-repair/default/build-windows.json) |
| Windows ROM gdbinit原先缺ESP_ROM_ELF_DIR | 设置实际ROM路径，13个新构建均生成符号文件 | [检查](windows-repair/cache-check.json) |
| 刚初始化Git还没有HEAD，10个工程版本检测失败 | 使用已有Git身份建立首次提交1e90483，重跑全部失败工程通过 | [失败快照](windows-repair/before-first-commit/build-windows.json)、[成功](windows-repair/default/build-windows.json) |
| Ubuntu旧RISC-V下载不完整 | 本轮先前串口任务完成全部Arduino工具安装，serial_info通过；后续全量编译通过 | [首次成功](linux-resume/arduino-first.json)、[全量](linux-resume/arduino-all.json) |
| Ubuntu OLED和environment_station下载U8g2超时 | 后续oled_clock完成同版本依赖安装，复用缓存补测两项通过 | [失败JSON](linux-resume/before-u8g2-retry/arduino-all.json)、[OLED失败日志](linux-resume/before-u8g2-retry/linux-oled.log)、[成功](linux-resume/arduino-all.json) |
| Linux辅助脚本误要求框架有version.txt | 新下载归档无该文件，改核对包元数据，环境检查及13项构建通过 | [原错误](linux-resume/idf/environment-before-package-check.log)、[成功检查](linux-resume/idf/environment.log) |
| Ubuntu框架没有Git历史，版本检查打印Git不可用警告 | 正常回退从源码读取v6.1.0，退出0并编译成功；这是非致命诊断 | [环境日志](linux-resume/idf/environment.log) |
| 辅助检查遇到virtualenv模块不存在、默认用户PIO缓存无写权限 | 使用标准venv成功建环境；PIO指定工作区缓存后命令正常，未删除或修改用户原缓存 | [IDF依赖安装](linux-resume/idf/python-install.log)及本次对话工具输出 |
| Git暂存检查发现七个公共config.h被旧规则忽略 | 22个工程统一改为忽略个人config.local.h，七个现有公共选择器纳入源码；固件源码未改动 |
| Git自动换行会改变清单哈希，原始日志有尾部空白 | 禁用自动字节转换、保留原始日志；清理四篇文档末尾多余空行后重新检查和打包 |

本轮主要失败都另存目录后重试，不覆盖其快照。更早的名称冲突、长路径、U8g2 2.36.19不可用、PIO封装IDF失败、SSH旧IP超时等，见[原审计表](AUDIT-2026-10-31.md)。部分更早尝试共用日志名，原始逐行输出曾被覆盖；不能宣称保存了历史每一次失败的完整原始日志。

## 复现入口及限制

Windows用[windows_idf.ps1](../scripts/windows_idf.ps1)，Ubuntu用[linux_idf.sh](../scripts/linux_idf.sh)。参数、版本及操作命令见[总报告](REPORT.md)。Linux工具框架来自PlatformIO包，由官方idf.py编译；不是EIM全新安装，且关闭了在线constraints检查，冻结依赖已保留。

[硬件规程](HARDWARE_RUNBOOK.md)和[本地Git/Actions说明](LOCAL_GIT_AND_CI.md)已补齐。GitHub Actions需要未来选定仓库并推送，结果出现在仓库Actions标签页，无需另建网页。本轮未执行这一步。
