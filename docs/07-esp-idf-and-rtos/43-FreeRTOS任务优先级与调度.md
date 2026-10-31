# 43 FreeRTOS任务优先级与调度

[返回总目录](../../README.md) · ESP32-D0WD / 4 MB Flash / ESP-IDF 6.1。

## 目标与消息模型

任务有独立栈、优先级和运行状态。就绪表示可竞争CPU，阻塞表示等待队列、时间或事件，挂起则不参与普通调度。双核仍只能让每核同一时刻执行一个任务上下文，不能使所有任务同时运行。

本例生产者每100ms生成序号和微秒时间戳，消费者从队列读取并计算延迟。这是调度实验事件，不是传感器读数。序号用来观察丢失和顺序；生成时间戳来自单调计时器，适合计算同一设备上的时间差。

## 周期与负载推导

若每轮工作耗时C，随后延迟T，实际周期约为C+T，工作时间会积累成漂移。vTaskDelayUntil以上一次计划唤醒点为基准，更适合周期任务；但执行超过周期时依然会错过截止时间。

假设单核三个任务执行时间为2、5、10ms，周期为20、100、200ms，平均负载为：

$$U=2/20+5/100+10/200=0.20.$$

20%只说明平均计算需求，不保证所有截止时间都能满足。锁等待、中断、系统任务和最长执行路径都会影响实时性，双核也不能简单把U除以二后给出保证。

## 优先级和阻塞

生产者优先级5、消费者4，数字需要结合系统其他任务理解。高优先级任务如果一直忙等、从不阻塞，会让低优先级任务饥饿。消费者在xQueueReceive上最多等待1秒，空队列时不占满CPU。

把while(!connected)空循环换成事件等待或有限重试，比盲目提高优先级更有效。周期采样可以用DelayUntil，等待不确定事件则用队列或通知，二者语义不同。

## 逐段阅读代码

sample_t包含uint32_t序号和int64_t生成时刻。队列长度8，按sizeof(sample_t)复制整个结构体。生产者发送超时为0，队列满就丢弃新事件并警告。消费者收到后立即记录当前时刻，差值包括排队与调度开销。

xTaskCreate返回值和队列创建结果必须检查，避免缺失任务后系统仍假装工作。本书按ESP-IDF接口将栈参数解释为字节，不能直接照搬其他FreeRTOS端口以字为单位的例子。3072字节是当前简单任务初始预算，复杂路径需要测量。

常驻任务使用带阻塞的循环，有限任务结束时按FreeRTOS要求删除自身，不从任务函数随意返回。大局部数组、深调用和格式化日志都消耗栈；高水位是已执行路径的证据，未触发分支仍需余量。

## 实验、验收与练习

正常运行记录100条延迟，报告中位数、P95、最大值和样本数。增加日志或Wi-Fi后再比较，说明日志本身也会干扰测量。

给消费者加250ms处理延时，生产约10条/秒、消费约4条/秒，积压约6条/秒，空队列大致1.3秒后满。增大容量只能缓冲突发，不能解决持续吞吐不足。恢复消费者速度后应继续接收新数据。

练习：每轮增加20ms工作，比较Delay和DelayUntil。参考：工作后延迟100ms得到约120ms周期；DelayUntil尝试维持100ms，实际仍有tick量化和调度抖动。

## 来源

- [IDF FreeRTOS](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/freertos_idf.html)
- [ESP Timer](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_timer.html)

## 完整工程与运行

[源码](../../examples/esp-idf/tasks_queue/main/main.c)，[构建配置](../../examples/esp-idf/tasks_queue/CMakeLists.txt)。先激活IDF 6.1，从手册根目录运行。

Windows PowerShell：

```powershell
Set-Location examples/esp-idf/tasks_queue
idf.py set-target esp32
idf.py build
$port = Read-Host '输入实际串口'
idf.py -p $port flash monitor
```

Linux Bash：

```bash
cd examples/esp-idf/tasks_queue
idf.py set-target esp32
idf.py build
read -r -p '输入实际串口: ' port
idf.py -p "$port" flash monitor
```

首次配置目标即可，已有配置先备份。退出monitor使用Ctrl+]。编译不要求接板，真实时序与波形需要硬件；[验证记录](../../validation/)列明已完成检查。前置：第40–42章；下一步按总目录继续。资料基线2026-10-29，教学代码为原创。
