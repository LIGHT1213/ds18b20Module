# <font face="微软雅黑">Letter驱动 STM32HAL 库版本
* 驱动说明  
移植自[Lettet Shell工程](https://github.com/NevermindZZT/letter-shell)
* 使用方式  
将对应的Shell中的文件直接加入编译，目前使用的是Uart1来作为数据交互入口，并在STM32CubeMx中加入freertos并开启一个用于通讯的线程并加入一个二进制信号量(这里我命名为shellBinarySem)，在freertos中定义全局变量  
> ```
> Shell shell;
>char tempData;
>uint8_t DataBuff[512];
> ```
同时在通讯线程中加入如下代码。  

> ```
>    osSemaphoreWait(shellBinarySemHandle, 10);
>    shell.write = shellWrite;
>    HAL_UART_Receive_DMA(&huart1,(uint8_t*)(&tempData),1);
>    shellInit(&shell,(char *)DataBuff,512);
>    /* Infinite loop */
>    for(;;)
>    {
>        if(osSemaphoreWait(shellBinarySemHandle, 10)==osOK)
>        {
>            shellHandler(&shell, tempData);
>        }
>        //osDelay(1);
>    }
> ```

* STM32CubeMX初始化说明  
 需要开启uart1的DMA
## *版本*
* 目前版本v0.1 
## *版本说明*
v0.1初始化文件
## *API使用说明*  
详细的请参考[这里](https://github.com/NevermindZZT/letter-shell/blob/master/README.md)