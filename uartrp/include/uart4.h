#ifndef __UART4_H_
#define __UART4_H_


#include "STC15W4K.h"      // 包含 "STC15W4K.H"寄存器定义头文件
#include	"config.h"

#define		TI4					(S4CON & 0x02) != 0
#define		RI4					(S4CON & 0x01) != 0
#define		SET_TI4()			S4CON |=  0x02
#define		CLR_TI4()			S4CON &= ~0x02
#define		CLR_RI4()			S4CON &= ~0x01

#define     S4TB8_SET()         (S4CON |= 0x08)
#define     S4TB8_CLR()         (S4CON &= ~0x08)


#define		UART4_INT_ENABLE()		IE2 |=  (1<<4)	
#define		UART4_INT_DISABLE()		IE2 &= ~(1<<4)	





extern void uart4_init( enum  ODD_EVEN oddEven);
extern void uart4_send(u8 *pbuf,u16 n);

 

#endif

