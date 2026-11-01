# GitHub 上传与 Actions 验证

更新日期：2026-11-01（北京时间）。仓库为 [MFSAWUY/ESP32_LearningRecord](https://github.com/MFSAWUY/ESP32_LearningRecord)，本地 main 已关联 origin/main。72 篇手册、源码及已有验证记录已上传；Actions 在仓库自带标签页运行，无需另建网页。本次未发布 GitHub Pages。

## 每次运行与修复

| 运行与提交 | 实际结果 | 原因及处理 |
|---|---|---|
| [41209496054](https://github.com/MFSAWUY/ESP32_LearningRecord/actions/runs/41209496054)，e5429b1 | failure，任务列表为空，没有进入编译 | job 级 env 不支持 runner 上下文；将 PLATFORMIO_CORE_DIR 移入构建步骤的 env。修复提交 9ec3e4e |
| [41229731984](https://github.com/MFSAWUY/ESP32_LearningRecord/actions/runs/41229731984)，9ec3e4e | 三个 job 均 success，全部编译/测试/容量检查通过；仅桌面两份产物上传成功 | IDF 报告使用主机临时路径，上传操作在容器映射路径查找，得到 No files 警告。改为相对目录并将无文件设置为 error |
| [41231159796](https://github.com/MFSAWUY/ESP32_LearningRecord/actions/runs/41231159796)，fd7b538 | 两个桌面 job 成功；IDF 13 次编译及容量检查成功，但上传步骤 failure | 新目录 .ci-validation/ 是隐藏目录，upload-artifact 默认过滤。修复提交 9b3f9b9 统一改成可见目录 ci-validation/ |
| [41306203116](https://github.com/MFSAWUY/ESP32_LearningRecord/actions/runs/41306203116)，9b3f9b9 | 三个 job 均 success，三份产物上传及下载校验通过 | 最新修复运行已完成；详见下方最终产物与源码哈希记录 |

根因参考：[GitHub 上下文可用范围](https://docs.github.com/en/actions/reference/workflows-and-actions/contexts#context-availability)、[upload-artifact v7 默认参数](https://github.com/actions/upload-artifact/blob/v7/action.yml)、[隐藏文件搜索实现](https://github.com/actions/upload-artifact/blob/v7/src/shared/search.ts)。普通 YAML 解析只检查语法，不能证明 Actions 语义正确；job 绿色也不足以证明报告确实上传。

## 已保存的证据

历史结果元数据见 [run-history.json](github-actions/run-history.json)，包含提交 SHA、各 job 和步骤结论。第二次运行已保存：

- [Ubuntu job 日志](github-actions/ubuntu-job.log)、[Windows job 日志](github-actions/windows-job.log)、[IDF job 日志](github-actions/idf-job.log)。
- [Ubuntu 构建报告](github-actions/ubuntu/arduino.json)、[Ubuntu 容量报告](github-actions/ubuntu/arduino-sizes.json)及同目录逐工程日志。
- [Windows 构建报告](github-actions/windows/arduino.json)、[Windows 容量报告](github-actions/windows/arduino-sizes.json)及同目录逐工程日志。
- 第三次运行的 [IDF 隐藏目录上传失败日志](github-actions/idf-hidden-directory-failure.log)。

第二次运行 Windows、Ubuntu 各 28 个 Arduino/综合工程编译成功，包含 ESP-NOW 两个环境，共 29 个镜像通过容量检查；各 9 项 Python 测试和 1 项 C++ 测试成功。两个构建报告各记录的 98 项源码哈希与当前源码一致。IDF 的 11 个默认工程与 2 种 OTA 配置编译、容量检查均成功，但该次缺失 IDF 下载产物，不能声称已完成报告交付。

已下载的第二次运行产物通过 SHA256 和 ZIP CRC 校验：

| 产物 | artifact ID | ZIP SHA256 |
|---|---|---|
| Ubuntu | 13053481983 | 87e3824055a7a467d6d3c72a1fbdf724ddda213ab4b89bd50329d43e9cb03a38 |
| Windows | 13053970728 | 3c4f166f3c4712738ac5c22d1707165e5bb805aed28be49aebf20e523bf3cd87 |

下载 ZIP 留在本地 .tools/ci-ubuntu.zip 与 .tools/ci-windows.zip；解压的报告/日志纳入版本管理和交付包。GitHub 在线产物按工作流保留 14 天，不能代替持久保存的验证记录。job 日志副本统一去除 BOM 并转换为 LF；产物内原始文件保留字节。

## 最终运行与已核验产物

运行 41306203116 使用提交 9b3f9b908647044b83942a713771c5346a84f3d8。整体结论 success，三个 job 全部成功，三份产物均实际下载核验。元数据见 [final-run.json](github-actions/final-run.json)：

| 环境 | 构建与测试 | 产物校验 |
|---|---|---|
| Windows Server2025 | 28个工程、29个镜像；Python 9项、C++ 1项通过 | 31个文件，98项源码哈希匹配，ZIP SHA256及CRC通过 |
| Ubuntu24.04 | 28个工程、29个镜像；Python 9项、C++ 1项通过 | 31个文件，98项源码哈希匹配，ZIP SHA256及CRC通过 |
| 官方IDF6.1容器 | 11个默认工程、联网OTA、自检失败OTA，共13次编译和容量检查通过 | 19个文件，73项源码哈希匹配，ZIP SHA256及CRC通过 |

[Windows校验摘要](github-actions/final-windows-audit.json)、[Ubuntu校验摘要](github-actions/final-ubuntu-audit.json)、[IDF校验摘要](github-actions/final-idf-audit.json)记录下载归档哈希、数量及检查结果。具体证据：[Windows日志](github-actions/final-windows-job.log)、[Ubuntu日志](github-actions/final-ubuntu-job.log)、[IDF日志](github-actions/final-idf-job.log)、[默认IDF构建](github-actions/final-idf/idf.json)、[联网OTA容量](github-actions/final-idf/ota/sizes.json)、[自检失败OTA容量](github-actions/final-idf/ota-fail/sizes.json)。

IDF产物共6个JSON和13份逐工程日志。所有镜像均通过4MiB布局检查，OTA双槽各1,966,080字节、回滚配置启用；联网镜像990,512字节、余量975,568字节，自检失败镜像989,872字节、余量976,208字节。构建报告与容量报告中的镜像哈希、字节数互相一致，公开配置overlay哈希也与当前文件匹配。这里仅核对报告一致性；下载产物没有包含固件二进制，实际.bin文件的容量/分区检查在runner内执行。

三个最终产物的 artifact ID、SHA256、保留期限见 final-run.json，ZIP原包保存在本地 .tools/ci-final-windows.zip、.tools/ci-final-ubuntu.zip、.tools/ci-final-idf.zip。解压后的报告与日志随手册同步；三个下载归档共81个文件。最终运行核对269项源码哈希（含跨环境和OTA配置的重复检查），全部匹配。

被测提交为9b3f9b9。之后的收尾提交仅补充说明、日志、报告和源码清单，使用 [skip ci] 避免重复耗时构建；固件、工作流和构建脚本保持与被测提交一致。不能将这个说明扩展为未来任意提交均已验证。

## 平台和验收边界

- Windows 托管 runner 实际为 Windows Server 2025，并非 Windows 11；本地 Windows 11 的证据仍见 [总报告](REPORT.md)。
- Ubuntu 托管 runner 为 Ubuntu 24.04；用户远程主机仍为 Ubuntu 22.04.5。两者的结果分别记录。
- IDF 托管任务使用 espressif/idf:v6.1 官方容器，调用 /opt/esp/idf/export.sh；它与本地由 PlatformIO 管理工具包的构建环境不同，也不等于 Windows EIM 全新安装验收。
- 公开 OTA 编译参数使用 example.invalid，仅用于保留联网代码路径。两种 OTA 配置的编译成功不证明真实联网升级或 bootloader 回滚。
- 硬件实验按用户安排暂缓。只枚举端口，不打开串口、不烧录；OLED、SHT30、网络恢复、NVS 重启、OTA 回滚、功耗和完整 JTAG 仍待设备连接。

## 上传连接与同步

本次 Windows 直连 GitHub 曾发生连接超时/SSL EOF。临时尝试仅监听 127.0.0.1:18532 的 SSH SOCKS 转发，部分推送成功、后续连接也超时；隧道已关闭，没有修改全局代理。一次工作流修复通过 GitHub 插件提交，随后正常 fetch 和 merge 保留双方提交历史，9b3f9b9 已通过原生 Git 推送。未使用强制推送，也未覆盖其他课程仓库。

Windows 源码在 D:\VSCode_data\C++\ESP32\handbook；Ubuntu 源码在 /home/pc/wty/ESP32/handbook，测试与工具在 /home/pc/wty/ESP32/test。最终同步使用 [sync_to_ubuntu.ps1](../scripts/sync_to_ubuntu.ps1)，先核对归档 SHA256，再核对逐文件清单；.git、工具链和构建缓存不随源码同步。更早下载/编译失败及恢复结果见 [10月31日后续记录](CONTINUATION-2026-10-31.md)。
