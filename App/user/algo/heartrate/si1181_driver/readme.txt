1. 接口函数放在si117x_config.h

2. 采样率固定16hz，读取周期4hz，可以改为2hz

3. 系统开机自检可以调用 Si117x_checkstart 返回0时，传感器正常

4. 调用 Si117x_Start 启动传感器，若返回0，传感器正常启动，非0则传感器异常，建议操作步骤如下
    if(Si117x_Start(OHRL_GetLEDcurrent()==0)
   {
       //启动成功
       //打开LED电源， 启动成功后再打开LED电源！
   }
  else
  { 
      //启动失败
  }
   

5. 调用 Si117x_Stop 传感器休眠

6. 再次调用 Si117x_Start 从休眠状态下唤醒传感器, 注意需要先调用 OHLR_start_configure()

7. HeartRateMonitor_interrupt_handle() 为fifo中断处理程序。
