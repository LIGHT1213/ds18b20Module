#include "ds18b20.h"
#include "string.h"
#include "stdio.h"
#include "log.h"
#define IO_IN 1
#define IO_OUT 0
static void setIoState(uint8_t state);
static uint8_t DS18B20_IO_Read();
static ds18b20_t *ds18b20List[DS18B20_MAX_SEARCH_SIZE]={NULL};
static void DS18B20_Search_Rom(void);
static void ds18b20Reset();
static void ds18b20GetTemp(ds18b20_t * thisDevice);
static void ds18b20Add(ds18b20_t *ds18b20);
static uint8_t ds18b20Cmp(uint8_t *romId);

static uint8_t ds18b20ReadBit(void);
static uint8_t ds18b20Read2Bit(void);
static uint8_t ds18b20ReadByte(void);
static void ds18b20WriteBit(uint8_t dat);
static void ds18b20WriteByte(uint8_t dat);
static uint8_t ds18b20AnswerCheck(void);

static uint8_t DS18B20_SensorNum;
static uint8_t DS18B20_ID[8][8];

static uint8_t DS18B20_IO_Read()
{
	if(DS18B20_Read()==GPIO_PIN_SET)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
uint8_t ds18b20Init()
{
	ds18b20Reset();
	while(ds18b20AnswerCheck())
	{}
	DS18B20_Search_Rom();
	for(int i=0;i<DS18B20_SensorNum;i++)
	{
		ds18b20_t *thisDs18b20;
		thisDs18b20=pvPortMalloc(sizeof(ds18b20_t));
		memcpy(thisDs18b20->romID,DS18B20_ID[i],8);
		ds18b20Add(thisDs18b20);
	}
	return 0;
}
static uint8_t ds18b20Cmp(uint8_t *romId)
{
		for(int i=0; i<DS18B20_SensorNum; i++)
		{
			if(memcmp(romId,ds18b20List[i]->romID,8)!=0)
			{
				return 1;
			}
		}
		return 0;
}
static void ds18b20Add(ds18b20_t *ds18b20)
{
    for(int i=0; i<DS18B20_MAX_SEARCH_SIZE; i++)
    {
        if(ds18b20List[i]==NULL)
        {
            ds18b20List[i]=ds18b20;
						break;
        }
    }
}
void ds18b20Task()
{
	ds18b20_t *thisDs18b20;
	DS18B20_Search_Rom();
	for(int i=0;i<DS18B20_SensorNum;i++)
	{
		
		if(ds18b20Cmp(DS18B20_ID[i])==1)
		{
			thisDs18b20=pvPortMalloc(sizeof(ds18b20_t));
			memcpy(thisDs18b20->romID,DS18B20_ID[i],8);
			ds18b20Add(thisDs18b20);
			vPortFree(thisDs18b20);
		}
		
		
//		logInfo("ID:%02x%02x%02x%02x%02x%02x%02x%02x TM:%.2f",thisDs18b20->romID[0],thisDs18b20->romID[1],thisDs18b20->romID[2],
//																																		thisDs18b20->romID[3],thisDs18b20->romID[4],thisDs18b20->romID[5],
//																																		thisDs18b20->romID[6],thisDs18b20->romID[7],thisDs18b20->temp);
		updateTemp();
	}
}
void updateTemp()
{
	for(int i=0; i<DS18B20_MAX_SEARCH_SIZE; i++)
	{
		if(ds18b20List[i]!=NULL)
		{
			ds18b20GetTemp(ds18b20List[i]);
		}
	}
}
static void ds18b20Reset()
{
	setIoState(IO_OUT);
	DS18B20_IO_Write_0();;		// 产生至少480us的低电平复位信号
	userUsDelay(480);
	DS18B20_IO_Write_1();;	// 在产生复位信号后，需将总线拉高
	userUsDelay(15);
	
}
static void setIoState(uint8_t state)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = DS18B20_IO_Pin;
	if(state==IO_IN)
	{
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	}
	else if(state==IO_OUT)
	{
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	}
	 GPIO_InitStruct.Pull = GPIO_PULLUP;
   HAL_GPIO_Init(DS18B20_IO_GPIO_Port, &GPIO_InitStruct);
}


// 从DS18B20读取1个位
static uint8_t ds18b20ReadBit(void)
{
	uint8_t data;
	setIoState(IO_OUT);
	DS18B20_IO_Write_0(); // 读时间的起始：必须由主机产生 >1us <15us 的低电平信号
	userUsDelay(2);
	DS18B20_IO_Write_1();
	userUsDelay(12);
	setIoState(IO_IN);// 设置成输入，释放总线，由外部上拉电阻将总线拉高
	if (DS18B20_IO_Read())
	{
		data = 1;
	}
	else
	{
		data = 0;
	}
	userUsDelay(50);
	return data;
}

// 从DS18B20读取2个位
static uint8_t ds18b20Read2Bit(void)//读二位 子程序
{

	uint8_t dat = 0;
	for (uint8_t i = 2; i > 0; i--)
	{
		dat = dat << 1;
		setIoState(IO_OUT);
		DS18B20_IO_Write_0();
		userUsDelay(2);
		DS18B20_IO_Write_1();
		setIoState(IO_IN);
		userUsDelay(12);
		if (DS18B20_IO_Read())	
		{
			dat |= 0x01;
		}
		userUsDelay(50);
	}
	return dat;
}

// 从DS18B20读取1个字节
static uint8_t ds18b20ReadByte(void)	// read one byte
{
	uint8_t j, dat=0;
	for (uint8_t i = 0; i < 8; i++)
	{
		j = ds18b20ReadBit();
		dat = (dat) | (j << i);
	}
	return dat;
}

// 写1位到DS18B20
static void ds18b20WriteBit(uint8_t dat)
{
	setIoState(IO_OUT);
	if (dat)
	{
		DS18B20_IO_Write_0();// Write 1
		userUsDelay(2);
		DS18B20_IO_Write_1();
		userUsDelay(60);
	}
	else
	{
		DS18B20_IO_Write_0();// Write 0
		userUsDelay(60);
		DS18B20_IO_Write_1();
		userUsDelay(2);
	}
}

// 写1字节到DS18B20
void ds18b20WriteByte(uint8_t dat)
{

	uint8_t testb;
	setIoState(IO_OUT);
	for (uint8_t j = 1; j <= 8; j++)
	{
		testb = dat & 0x01;
		dat = dat >> 1;
		if (testb)
		{
			DS18B20_IO_Write_0();// 写1
			userUsDelay(10);
			DS18B20_IO_Write_1();
			userUsDelay(50);
		}
		else
		{
			DS18B20_IO_Write_0();// 写0
			userUsDelay(60);
			DS18B20_IO_Write_1();// 释放总线
			userUsDelay(2);
		}
	}
}


void DS18B20_Search_Rom(void)
{
	uint8_t dsBack, l, errorBit, num=0;
	uint8_t stack[5];
	uint8_t overStack[64];
	uint8_t tempp;
	l = 0;
	memset(stack,0,sizeof(stack));
	memset(overStack,0,sizeof(overStack));
	do
	{
		ds18b20Reset(); //注意：复位的延时不够
		ds18b20AnswerCheck();
		userUsDelay(720); //480、720
		
		ds18b20WriteByte(0xF0);
		for (uint8_t m = 0; m < 8; m++)
		{
			uint8_t s = 0;
			for (uint8_t n = 0; n < 8; n++)
			{
				dsBack = ds18b20Read2Bit();//读两位数据

				dsBack = dsBack & 0x03;
				s >>= 1;
				if (dsBack == 0x01)//01读到的数据为0 写0 此位为0的器件响应
				{
					ds18b20WriteBit(0);
					overStack[(m * 8 + n)] = 0;
				}
				else if (dsBack == 0x02)//读到的数据为1 写1 此位为1的器件响应
				{
					s = s | 0x80;
					ds18b20WriteBit(1);
					overStack[(m * 8 + n)] = 1;
				}
				else if (dsBack == 0x00)//读到的数据为00 有冲突位 判断冲突位
				{
					//如果冲突位大于栈顶写0 小于栈顶写以前数据 等于栈顶写1
					errorBit = m * 8 + n + 1;
					if (errorBit > stack[l])
					{
						ds18b20WriteBit(0);
						overStack[(m * 8 + n)] = 0;
						stack[++l] = errorBit;
					}
					else if (errorBit < stack[l])
					{
						s = s | ((overStack[(m * 8 + n)] & 0x01) << 7);
						ds18b20WriteBit(overStack[(m * 8 + n)]);
					}
					else if (errorBit == stack[l])
					{
						s = s | 0x80;
						ds18b20WriteBit(1);
						overStack[(m * 8 + n)] = 1;
						l = l - 1;
					}
				}
				else
				{
					//没有搜索到
				}
			}
			tempp = s;
			DS18B20_ID[num][m] = tempp; // 保存搜索到的ID
		}
		num = num + 1;// 保存搜索到的个数

	} while (stack[l] != 0 && (num < DS18B20_MAX_SEARCH_SIZE));
	DS18B20_SensorNum = num;
}
static uint8_t ds18b20AnswerCheck(void)
{
	uint8_t delay = 0;
	setIoState(IO_IN); // 主机设置为上拉输入
	// 等待应答脉冲（一个60~240us的低电平信号 ）的到来
	// 如果100us内，没有应答脉冲，退出函数，注意：从机接收到主机的复位信号后，会在15~60us后给主机发一个存在脉冲
	while (DS18B20_IO_Read()&&delay < 100)
	{
		delay++;
		userUsDelay(1);
	}
	// 经过100us后，如果没有应答脉冲，退出函数
	if (delay >= 100)//Hu200
		return 1;
	else
		delay = 0;
	// 有应答脉冲，且存在时间不超过240us
	while (!DS18B20_IO_Read()&&delay < 240)
	{
		delay++;
		userUsDelay(1);
	}
	if (delay >= 240)
		return 1;
	return 0;
}
static void ds18b20GetTemp(ds18b20_t * thisDevice)
{
	//u8 flag;
	uint8_t j;//匹配的字节
	uint8_t TL, TH;
	short tempV;
	float Temperature;
	ds18b20Reset();
	ds18b20AnswerCheck();
	ds18b20WriteByte(0xcc);// skip rom
	ds18b20WriteByte(0x44);// convert
	ds18b20Reset();
	ds18b20AnswerCheck();

	ds18b20WriteByte(0x55);
	for (j = 0; j < 8; j++)
	{
		ds18b20WriteByte(thisDevice->romID[j]);
	}
	ds18b20WriteByte(0xbe);// convert
	TL = ds18b20ReadByte(); // LSB
	TH = ds18b20ReadByte(); // MSB
	if (TH & 0xfc)
	{
		//flag=1;
		tempV = (TH << 8) | TL;
		Temperature = (~tempV) + 1;
		Temperature *= 0.0625;
	}
	else
	{
		//flag=0;
		Temperature = ((TH << 8) | TL)*0.0625;
	}
	thisDevice->temp= Temperature;
}
