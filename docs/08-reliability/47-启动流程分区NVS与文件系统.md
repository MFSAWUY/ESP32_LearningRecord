# 47 启动流程分区NVS与文件系统

[返回总目录](../../README.md)

串口先枚举再选择；本章端口变量的初始化见[Windows/Linux公共命令](../../examples/esp-idf/README.md)。

## 从复位到app_main

前置：第02、17、41章。完整工程：[nvs_storage](../../examples/esp-idf/nvs_storage/)，文件日志例见[environment_station](../../examples/projects/environment_station/)。芯片复位后先执行ROM引导逻辑，根据启动配置进入下载模式或从Flash读取引导程序；第二阶段引导根据分区表和OTA状态选择应用，初始化系统后进入app_main。Flash存储空间不是全部留给你的C数组，运行时RAM与持久化存储也不能混为一谈。

分区表给出名字、类型、偏移和大小。常见分区包括NVS、OTA状态、应用槽和文件系统。偏移必须符合该分区类型的对齐要求，所有分区不能重叠且终点不能超过4MB。修改分区布局后，旧数据可能落在新分区的错误位置，应制定迁移或备份策略，而不是仅重新烧录应用就假设兼容。

## NVS键值模型

NVS适合少量配置项、计数器和校准参数，不适合高频连续大日志。示例命名空间handbook、键boots、类型uint32_t。nvs_flash_init失败时明确报告并保留数据，不自动擦除整个NVS。nvs_get_u32返回NOT_FOUND时使用首次启动默认0；其他错误不能一概当作不存在。写入调用nvs_set_u32后还要nvs_commit，提交成功才报告持久化成功，最后关闭句柄。

计数器达到UINT32_MAX后饱和，不回绕成0，避免误解为新设备。这个策略只适用于教学启动计数；真实业务应定义溢出、重置权限和掉电语义。单次启动写一次尚可，但每毫秒持久化会增加写放大和擦写压力。NVS内部的磨损均衡不意味着Flash无限寿命。

## 两端运行与掉电实验

在IDF6.1终端进入工程：两端先执行`idf.py set-target esp32`、`idf.py build`。Windows上传`idf.py -p $serialPort flash monitor`，Linux上传`idf.py -p "$serial_port" flash monitor`。首次计数1，按EN后增加；重新下载应用通常不清除NVS，但全片擦除会清除。不要用断电发生在未提交之前的结果去断言所有写入都会丢失。

## 文件系统与容量预算

SPIFFS、LittleFS和FAT有不同空间、磨损和功能特征，文件API相似不等于底层格式兼容。IDF组件依赖需固定版本；本手册Arduino环境监测用LittleFS，SD卡示例使用对应卡文件系统。文件系统挂载失败不应默默格式化用户数据，首次初始化必须明确。

假设一条记录128字节，每秒1条，日增长128×86400≈10.55MiB。仅4MB Flash无法保存一天，应滚动日志或转移到SD/服务器。每次写入后flush增加持久性但也有性能成本；缓存10条提高吞吐，却可能在突然断电时丢失末尾部分。需要对记录加序号和校验，启动时识别截断尾部。

验收：首次启动、连续复位、断电恢复、键类型不一致、分区不存在分别有可解释结果。练习：增加配置schema_version。参考：升级时读取旧版本，转换到新结构并确认提交，再切换版本标记；不识别的未来版本应保留而不是强制重置。来源：[NVS文档](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/storage/nvs_flash.html)。下一步：第48章。
