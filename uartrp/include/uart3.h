#ifndef __UART3_H_
#define __UART3_H_


#include "STC15W4K.h"      // 包含 "STC15W4K.H"寄存器定义头文件
#include	"config.h"


#define		TI3					          (S3CON & 0x02) != 0
#define		RI3					          (S3CON & 0x01) != 0
#define		SET_TI3()			          S3CON |=  0x02
#define		CLR_TI3()			          S3CON &= ~0x02
#define		CLR_RI3()			          S3CON &= ~0x01

#define     S3TB8_SET()                   (S3CON |= 0x08)
#define     S3TB8_CLR()                   (S3CON &= ~0x08)

#define		UART3_INT_ENABLE()		IE2 |=  (1<<3)	
#define		UART3_INT_DISABLE()		IE2 &= ~(1<<3)	

extern void uart3_init(u32 baud, enum  ODD_EVEN oddEven);
extern void uart3_send(u8 *pbuf,u16 n);


#endif

