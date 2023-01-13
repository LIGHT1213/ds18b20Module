#ifndef SHELL_PORT_H
#define SHELL_PORT_H
#include "main.h"
#include "usart.h"
#include "shell.h"
#include "freertos.h"
/**
 * @brief shell读取数据函数原型
 *
 * @param char shell读取的字符
 *
 * @return char 0 读取数据成功
 * @return char -1 读取数据失败
 */
static short shellWrite(char* data,unsigned short len);
static short shellRead(char* data,unsigned short len);
void userShellInit(void);

#endif