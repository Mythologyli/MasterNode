## LoRa-02 

基于 [wdomski/SX1278](https://github.com/wdomski/SX1278) 库封装的简单的 LoRa 模块操作库。

由于实验板在设计时忽略了 DIO0 引脚的引出，导致无法使用 DIO0 上的接收、发送中断。因此，对 wdomski/SX1278 库进行了修改，采用检测 FifoRxCurrentaddr 是否移动的方法判断是否有新的接收。在发送时不检测发送中断。

本实现采用了手册上未定义的方法，可能出现各种问题，建议在可能的情况下引出 DIO0 使用。
