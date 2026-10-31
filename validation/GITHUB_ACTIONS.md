# GitHub 上传与 Actions 验证

2026-11-01更新。用户已指定仓库[MFSAWUY/ESP32_LearningRecord](https://github.com/MFSAWUY/ESP32_LearningRecord)，本地main已推送并关联origin/main。原本“尚未上传”的记录属于10月31日上传前快照。

## 首次运行

[运行41209496054](https://github.com/MFSAWUY/ESP32_LearningRecord/actions/runs/41209496054)对应提交e5429b1，结论failure，任务列表为空，未进入源码构建。

排查发现job级env使用了runner.temp，而该位置不允许runner上下文。修复将PLATFORMIO_CORE_DIR移到Arduino构建步骤的env中；参考[GitHub官方上下文表](https://docs.github.com/en/actions/reference/workflows-and-actions/contexts#context-availability)。普通YAML解析不能发现这种Actions语义错误，之前的本地解析记录不代表GitHub已经接受工作流。

修复后须以新运行的真实job结果为准，本页后续补充。硬件实验仍按用户安排暂缓；托管runner不连接用户开发板。
