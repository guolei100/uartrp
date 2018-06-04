#ifndef __UART1_H_
#define __UART1_H_

#include "STC15W4K.h"      // 包含 "STC15W4K.H"寄存器定义头文件
#include	"config.h"
#include "uart_repeater.h"


#define		S1_USE_P30P31()		P_SW1 &= ~0xc0					//UART1 Ê¹ÓÃP30 P31¿Ú	Ä¬ÈÏ
#define		S1_USE_P36P37()		P_SW1 = (P_SW1 & ~0xc0) | 0x40	//UART1 Ê¹ÓÃP36 P37¿Ú
#define		S1_USE_P16P17()		P_SW1 = (P_SW1 & ~0xc0) | 0x80	//UART1 Ê¹ÓÃP16 P17¿Ú






extern u8  uart1RevBuf[REV_BUFF_LEN];
extern u16 uart1RevLen ;

extern void uart1_init( enum  ODD_EVEN oddEven);
extern void uart1_send(u8 *pbuf,u16 n);
extern char putchar(char c);
extern void UART1_SendString(char *s);

extern void start_timer1(u8 countOf5ms,int (*pcallback)(void ));
extern void stop_timer1(void);

#endif

