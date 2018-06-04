#ifndef __UART2_H_
#define __UART2_H_

#include "STC15W4K.h"      // 包含 "STC15W4K.H"寄存器定义头文件
#include	"config.h"


#define		TI2					          (S2CON & 0x02) != 0
#define		RI2					          (S2CON & 0x01) != 0
#define		SET_TI2()			          S2CON |=  0x02
#define		CLR_TI2()			          S2CON &= ~0x02
#define		CLR_RI2()			          S2CON &= ~0x01

#define     S2TB8_SET()                   (S2CON |= 0x08)
#define     S2TB8_CLR()                   (S2CON &= ~0x08)


#define		UART2_INT_ENABLE()		IE2 |=  0x01	
#define		UART2_INT_DISABLE()		IE2 &= ~0x01	


extern void uart2_init(u32 baud, enum  ODD_EVEN oddEven);
extern void uart2_send(u8 *pbuf,u16 n);




#endif

