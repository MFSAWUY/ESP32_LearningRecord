# GitHub 上传与 Actions 验证

2026-11-01更新。用户已指定仓库[MFSAWUY/ESP32_LearningRecord](https://github.com/MFSAWUY/ESP32_LearningRecord)，本地main已推送并关联origin/main。原本“尚未上传”的记录属于10月31日上传前快照。

## 首次运行

[运行41209496054](https://github.com/MFSAWUY/ESP32_LearningRecord/actions/runs/41209496054)对应提交e5429b1，结论failure，任务列表为空，未进入源码构建。

排查发现job级env使用了runner.temp，而该位置不允许runner上下文。修复将PLATFORMIO_CORE_DIR移到Arduino构建步骤的env中；参考[GitHub官方上下文表](https://docs.github.com/en/actions/reference/workflows-and-actions/contexts#context-availability)。普通YAML解析不能发现这种Actions语义错误，之前的本地解析记录不代表GitHub已经接受工作流。

修复提交9ec3e4e已推送，[第二次运行41229731984](https://github.com/MFSAWUY/ESP32_LearningRecord/actions/runs/41229731984)成功启动了Windows桌面、Ubuntu24.04桌面及IDF容器三个任务。结果将在完成后补充，不能把任务启动当作编译通过。

Windows直连GitHub数次连接超时，但现有Ubuntu可访问。推送时使用仅监听本机的临时SSH SOCKS转发，并仅对该条Git命令指定代理；未设置全局代理。原有提交历史保留。

第二次运行的IDF编译与容量检查通过，但产物上传警告未找到文件：run步骤把runner.temp展开为主机路径，而上传操作在容器映射路径查找。已改为工作区内的相对目录`.ci-validation/`，并把IDF上传的if-no-files-found改为error。需再次运行验证真正上传，而非只接受任务绿色状态。

## 阅读Actions结果

进入仓库的Actions标签页，选择validate-handbook及对应提交。每个job展开步骤后可查日志；失败时先确认是工作流语义、runner/下载环境、编译还是容量检查。完整日志和validation产物在任务结束后提供。下载产物只能证明对应提交与该运行的检查结果，不代表后续代码自动验证。

Windows托管runner的系统版本以其日志为准，不能将windows-latest称为Windows11实测。IDF使用espressif/idf:v6.1容器。硬件实验仍按用户安排暂缓；托管runner不连接用户开发板。
