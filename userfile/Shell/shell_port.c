#include "main.h"
#include "shell_port.h"
#include "shell.h"
static uint8_t isShellOpen;
static uint8_t DataBuff[256];  //shell 用缓冲区

char tempData;
void commandProcess(void);


void switchToShell();
void getEeprom(uint8_t page);
void laser_cur_eeprom(void);
void uartLogWrite(char *buffer, short len);

uint8_t rbBuff[200];
ring_buffer RB;


Log uartLog = {
    .write = uartLogWrite,
    .active = 0x01,
    .level = LOG_DEBUG
};
Shell shell;

void userShellInit(void)
{
		Ring_Buffer_Init(&RB,rbBuff,200);
    shell.write = shellWrite;
#ifndef OLD_SHELL
    shell.read = shellRead;
#endif

    shellInit(&shell,(char *)DataBuff,256); //shell初始化

    logRegister(&uartLog, &shell); //shell伴生对象初始化
		logSetLevel(&uartLog,LOG_INFO);
    HAL_UART_Receive_IT(&huart1,(uint8_t*)(&tempData),1);
}
static short shellWrite(char* data,unsigned short len)
{
    if(osSemaphoreWait(uartSendBinarySemHandle,osWaitForever)==osOK)
    {
        HAL_UART_Transmit_DMA(&huart1, (uint8_t *)data, len);
        return len;
    }
    else
    {
        return 0;
    }
}
static short shellRead(char* data,unsigned short len)
{
    unsigned short curDataLength=Ring_Buffer_Get_Lenght(&RB);
    if(curDataLength>0 && Ring_Buffer_Get_Lenght(&RB)>=len)
    {
        Ring_Buffer_Read_String(&RB,(uint8_t*)data,len);
        return len;
    }
    else if(curDataLength>0 && Ring_Buffer_Get_Lenght(&RB)<len)
    {
        Ring_Buffer_Read_String(&RB,(uint8_t*)data,curDataLength);
        return curDataLength;
    }
    else
    {
        return 0;
    }
}
void uartLogWrite(char *buffer, short len)
{    
	if (uartLog.shell)
  {
    shellWriteEndLine(&shell,buffer,len);
	}
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &huart1)
    {
        osSemaphoreRelease(uartSendBinarySemHandle);
    }
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

    if (huart == &huart1)
    {
#ifdef OLD_SHELL
        osSemaphoreRelease(shellBinarySemHandle);
        //shellHandler(&shell, tempData);
        HAL_UART_Receive_IT(&huart1, (uint8_t *)(&tempData), 1);
#else

        Ring_Buffer_Write_Byte(&RB, tempData);

        HAL_UART_Receive_IT(&huart1, (uint8_t *)(&tempData), 1);
#endif
        //ReceiveFlag=1;
    }

}
