# 验证报告

更新日期：2026-10-31。Windows旧映射缓存已通过真实路径新目录重建替代；Ubuntu工具链已安装，全部固件工程实际编译通过。最新过程与日志位置见[本轮记录](CONTINUATION-2026-10-31.md)，当日早先状态保留为[历史快照](AUDIT-2026-10-31.md)。

2026-11-01后续：用户已指定GitHub仓库，手册已推送并启用Actions；真实运行状态见[GitHub验证记录](GITHUB_ACTIONS.md)。真机实验仍按用户安排暂缓，不能用编译成功代替。

## 交付与实际结果

72篇正文，28个Arduino/PlatformIO工程（23个基础示例、5个综合项目）、11个原生ESP-IDF工程，配套Python上位机、数学仿真、参数模板和验收说明。ESP-NOW网关工程有网关、节点两个环境。

| 项目 | 结果 | 证据 |
|---|---|---|
| 72章、本地链接、围栏、Python语法 | 静态检查通过 | [结构检查](structure-windows.json) |
| Windows Arduino/综合工程 | 历史28/28通过、29个镜像，本轮未全量重编 | [构建](build-windows.json)、[大小](firmware-sizes.json) |
| Ubuntu Arduino/综合工程 | 本轮28/28通过、29个镜像 | [构建](linux-resume/arduino-all.json)、[大小](linux-resume/arduino-sizes.json) |
| Windows原生IDF默认配置 | 新真实路径11/11通过 | [构建](windows-repair/default/build-windows.json)、[大小](windows-repair/default/sizes.json) |
| Ubuntu原生IDF默认配置 | 11/11通过 | [构建](linux-resume/idf/default/build-linux.json)、[大小](linux-resume/idf/default/sizes.json) |
| Windows联网OTA / 自检失败OTA | 两种配置的编译和容量检查通过 | [联网](windows-repair/ota-configured/build-windows.json)、[自检失败](windows-repair/ota-selftest-fail/build-windows.json) |
| Ubuntu联网OTA / 自检失败OTA | 两种配置的编译和容量检查通过 | [联网](linux-resume/idf/ota/build-linux.json)、[自检失败](linux-resume/idf/ota-fail/build-linux.json) |
| Windows新缓存、ROM符号辅助文件 | 13个构建缓存无已撤销映射，ROM符号文件生成 | [检查](windows-repair/cache-check.json) |
| Windows / Ubuntu Python | 各9项通过 | [Windows](python-windows-resume.log)、[Ubuntu](linux-resume/python-linux.log) |
| Windows / Ubuntu桌面C++ | 各1项通过 | [Windows](cpp-windows-test.log)、[Ubuntu](cpp-linux-test.log) |
| GitHub Actions | 已上传并启用；运行状态独立记录 | [运行记录](GITHUB_ACTIONS.md)、[配置](../.github/workflows/validate.yml) |
| 真机、OLED、传感器、网络恢复、NVS、OTA回滚、功耗、完整JTAG | 按用户安排暂不执行 | [规程](HARDWARE_RUNBOOK.md)、[验收表](HARDWARE_CHECKLIST.md) |

Python测试覆盖RC、滤波、混叠、功耗、PID限幅/积分约束、数据协议、HTTP成功/503故障、pySerial软件回环与CSV；HTTP503属于预期故障注入。C++测试覆盖输入边界、非法命令不改变配置、宽位累计与重置。仿真、软件回环和交叉编译均不证明实际电路和设备正常。

## 实际系统与依赖

| 项目 | 实际版本 |
|---|---|
| Windows | Windows11 x64，构建10.0.26300，Python3.12.8 |
| Ubuntu | 22.04.5 LTS x86_64，内核6.8.0-138-generic，独立venv Python3.12.9 |
| 文档/CI目标 | Ubuntu24.04；不能把22.04实测当作24.04已经执行 |
| PlatformIO Core / 平台 | 6.1.18 / Espressif32 8.0.0 |
| Arduino核心 | Arduino-ESP32 3.3.12，GCC14.2 |
| Arduino外部库 | U8g2 2.36.18、PubSubClient2.8、WebSockets2.6.1 |
| 原生IDF | ESP-IDF6.1.0，Xtensa GCC15.2.0+20251204 |
| Windows CMake / Ninja | 3.30.2 / 1.9.0 |
| Ubuntu CMake / Ninja | 3.22.1 / 1.10.1 |
| 桌面C++ | Windows MSVC19.29；Ubuntu GNU11.4 |
| 桌面Python直接依赖 | pyserial3.5、paho-mqtt2.1.0、matplotlib3.9.2 |

[Windows原生环境](windows-repair/environment.log)、[Ubuntu原生环境](linux-resume/idf/environment.log)、[Ubuntu IDF依赖冻结](linux-resume/idf/requirements-frozen.txt)、[安装日志](linux-resume/idf/python-install.log)记录真实版本。桌面冻结清单另见[Windows](requirements-windows.txt)、[Ubuntu](requirements-linux.txt)。

原生构建调用官方tools/idf.py，框架及编译器由PlatformIO下载管理；这是包环境下的实际构建验证，**不是EIM全新安装验收**。依赖按框架requirements.core安装，IDF_PYTHON_CHECK_CONSTRAINTS=no关闭在线constraints检查，不能宣称严格复现EIM锁定依赖。Ubuntu IDF与Arduino使用不同缓存和venv。

Ubuntu框架归档不含Git历史，idf.py先打印Git不可用警告，再从源码读出v6.1.0并退出0；原输出已保留。这不是源码编译失败。Windows旧框架缓存含PlatformIO生成的__pio_env空组件，交付原生工程未新增PlatformIO配置。

## 容量和调试边界

两端联网OTA镜像均990,512字节，每槽1,966,080字节，余量975,568字节；自检失败镜像989,872字节，余量976,208字节。默认空网络配置镜像172,448字节，网络路径可能被优化掉，因此额外检查非空网络参数和自检失败配置。分区无重叠、未超过4MiB，回滚选项已启用。

公开编译参数使用example.invalid，不能直接用于上板联网。真实升级还需有效服务器、证书、时间、不同固件版本和已确认基线。编译自检失败分支不等于bootloader完成过真机回滚。

新缓存的ROM gdbinit文件已正常生成，不再出现旧ESP_ROM_ELF_DIR缺失问题；未验证外置探针、驱动、OpenOCD/GDB连接、断点和单步，完整JTAG保持待验收。

## 复现与同步

已安装环境的Windows入口：

```powershell
.\scripts\windows_idf.ps1 -Mode Check
.\scripts\windows_idf.ps1 -Mode BuildAll -Parallel 2
.\scripts\windows_idf.ps1 -Mode Ota
.\scripts\windows_idf.ps1 -Mode OtaSelftestFail
```

Ubuntu入口：

```bash
cd /home/pc/wty/ESP32/handbook
bash scripts/linux_idf.sh check
bash scripts/linux_idf.sh default
bash scripts/linux_idf.sh ota
bash scripts/linux_idf.sh ota-fail
```

两个助手仅面向本次已安装的包环境，不负责首次安装，不会烧录。Windows仅在脚本执行期间设置并恢复工具变量，不创建映射；Linux变量只影响该Bash子进程。重复执行会更新同名当前日志，要保留某次原始输出时先复制记录目录。--retry-failed跳过旧成功项，源码修改后应重建受影响项目。

交付包排除工具链、构建缓存、个人参数和私钥。原93个外链的历史结果为91可达、0确认失效、2连接受限，本轮未重查全部外链，见[记录](external-links.json)。源文件检查：

```text
python scripts/check_handbook.py
python scripts/verify_manifest.py
python -m unittest discover -s examples/python/tests -v
```

远端源码在/home/pc/wty/ESP32/handbook，环境/缓存/测试在/home/pc/wty/ESP32/test。同步先核对压缩包SHA256，再解压核验文件清单；不复制.git或工具缓存：

```powershell
.\scripts\sync_to_ubuntu.ps1 -Python '.\.tools\venv\Scripts\python.exe'
```

本地Git现已关联并推送到origin：MFSAWUY/ESP32_LearningRecord。Ubuntu文件同步不代表在Ubuntu初始化了同一Git仓库。
