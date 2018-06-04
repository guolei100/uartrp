#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "STC15W4K.h"      // 包含 "STC15W4K.H"寄存器定义头文件
#include	"config.h"
#include    <stdio.h>

#define   OPEN_DEBUG      1


void SendString(u8 *s);

#if   OPEN_DEBUG
//#define DEBUG(format, ...) printf (format, ##__VA_ARGS__)
//#define DEBUG(format,...) printf("File: "__FILE__", Line: %05d: "format"/n", __LINE__, ##__VA_ARGS__)
#define DEBUG    printf
//#define DEBUG(...)  printf( __VA_ARGS__)




#else
#define DEBUG(X)   
#endif
#endif

