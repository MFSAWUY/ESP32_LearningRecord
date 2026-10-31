# 本地 Git 与 GitHub Actions 的区别

**2026-11-01后续状态：** 用户已指定[ESP32_LearningRecord](https://github.com/MFSAWUY/ESP32_LearningRecord)，本地main已推送并关联origin/main，Actions已启用。实际运行记录见[GitHub验证](GITHUB_ACTIONS.md)。下文10月31日说明保留为上传前阶段记录。

2026-10-31：已按用户要求在 Windows 手册根目录初始化本地 Git，初始分支为 `main`。没有设置远程地址，没有上传到 GitHub，也没有运行 GitHub Actions。首次本地提交 `1e90483` 保存忽略规则、换行规则和许可证；提交使用电脑上已有的 Git 身份配置，没有修改全局身份。

刚初始化但没有提交的仓库曾使 ESP-IDF 的 Git 版本探测报 `git-data/head-ref` 不存在，建立有效 HEAD 后重新构建。本轮在验证和日志汇总后保存完整源码快照；之后新增改动是否已提交，以 `git status` 和 `git log` 的实际结果为准。

## 现在可以做什么

在本手册根目录，PowerShell 与 Bash 的 Git 命令相同：

```text
git status --short
git branch --show-current
git remote -v
git check-ignore .tools/venv/pyvenv.cfg ESP32-handbook.zip
```

本地 Git 保存代码版本；它不需要 GitHub 账号或联网。当前 `.gitignore` 排除了 `.tools/`、`.pio/`、构建目录、虚拟环境、个人 `config.local.h`、`sdkconfig`、证书私钥等；源码、公共参数模板、文档及验证记录可纳入版本管理。提交之前仍应检查差异和新增文件，忽略规则不能识别任意位置的密码。

七个工程的公共`config.h`仅负责选择公共模板或个人`config.local.h`，必须纳入版本管理；本轮已修正其旧忽略规则。`.gitattributes`禁用自动CRLF/LF转换，以保留源码清单及历史日志的原始字节哈希；Bash脚本保持LF。原始日志的尾部空白不会为了格式检查被删除。

需要保存或更新源码版本时，先确认自己的 Git 提交身份。如果电脑尚未配置，可以只为这个仓库设置，不影响其他工程：

```text
git config user.name "你的提交名称"
git config user.email "你的提交邮箱或GitHub noreply邮箱"
git add .gitattributes .gitignore .github LICENSE README.md assets docs examples scripts validation
git diff --cached --stat
git diff --cached --check
git commit -m "Add ESP32 handbook and validation tools"
```

上面的身份值需要换成自己的信息；已有正确身份时无需重新配置。这些命令是复现说明，不代表每个后续改动都已自动提交，实际状态以 `git status` 和 `git log` 为准。

## GitHub Actions 在哪里运行

[工作流文件](../.github/workflows/validate.yml)描述 GitHub 托管 runner 应执行的检查。工作流随源码推送到选定的 GitHub 仓库后，按 `push`、`pull_request` 或 `workflow_dispatch` 触发。GitHub 会在该仓库的 **Actions** 标签页显示任务、日志和下载产物，不需要创建网页。

**GitHub Pages** 是另外的静态网站发布功能；学习手册及运行 Actions 并不要求开启 Pages。本次没有创建 GitHub 仓库、上传源代码或发布网站。

后续决定仓库后，再配置远程地址和推送。不要把已有的其他课程仓库当作默认目标，也不要覆盖其文件。Actions 的 Windows runner 可能是 Windows Server，实际版本应以运行日志为准，不能据此声称 Windows 11 真机再次通过。

## 验证证据

本地可以检查工作流语法、运行同样的 Python/固件构建命令；这些属于本地验证。只有真实 GitHub run 的链接、commit SHA、各 job 结果和上传日志，才能作为“Actions 执行通过”的证据。静态检查或工作流文件存在都不能代替运行结果。

硬件端不会自动出现在托管 runner 上。即使桌面测试和交叉编译全部通过，OLED、传感器、OTA 回滚、功耗与 JTAG 仍须在连接实际设备后另行验收。

本轮工作流将新生成的报告放入 runner 临时目录，与仓库内历史结果分开；增加了C++桌面测试、实际镜像分区检查和非空联网参数OTA构建。当前使用的v7操作已核对官方发布记录：[checkout](https://github.com/actions/checkout/releases)、[setup-python](https://github.com/actions/setup-python/releases)、[upload-artifact](https://github.com/actions/upload-artifact/releases)。这里只确认操作版本存在及本地YAML可解析，尚无GitHub run结果。
