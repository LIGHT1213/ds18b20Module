#ifndef __DS18B20_H
#define __DS18B20_H
#include "main.h"
/**
 * @brief ds18b20 最大设备数量
 */
#ifndef DS18B20_MAX_SEARCH_SIZE
    #define DS18B20_MAX_SEARCH_SIZE 8
#endif

typedef struct{
	uint8_t binData[9];
	float temp;
	uint8_t resolution;
	uint8_t romID[8];
		
}ds18b20_t;
#define DS18B20_Read() HAL_GPIO_ReadPin(DS18B20_IO_GPIO_Port, DS18B20_IO_Pin)
#define DS18B20_IO_Write_1() HAL_GPIO_WritePin(DS18B20_IO_GPIO_Port, DS18B20_IO_Pin, GPIO_PIN_SET)
#define DS18B20_IO_Write_0() HAL_GPIO_WritePin(DS18B20_IO_GPIO_Port, DS18B20_IO_Pin, GPIO_PIN_RESET)
#define WITE_TEMP_TRAN() osDelay(800)
void ds18b20Task();
uint8_t ds18b20Init();
void updateTemp();
#endif