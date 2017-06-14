### 本文用于描述Dthing系统由于各种原因中添加的workaround行为.随着系统稳定性的前进,后续需要消除这些workaround
--------

1. Dthing用于E电项目的重启机制
Why: 由于Dthing稳定性不够, 在E电的dthing App中,需要不停的读串口和从MQTT发消息, 可能遇见一些意外情况(dthing本身的bug,待解), 很长时间都不读写串口和发送MQTT,我们就用这个行为的发生时间戳判断,如果过于约定时间没有行为发生,重启设备以规避dthing app陷入死等!
How: 每次度串口和发送mqtt时创建并记录调用时间戳，创建定时器,定期去查询时间戳和当前时间戳差, 大于约定时间后 重启系统
Keywords: [dthing-workaround-nix-1]
Related Files: 
Created Date: 2016.11.08
Create By: nix.long
Fixed: NO (本字段及以下字段等Fix后填写)
Fixed by:
Fixed Date:



