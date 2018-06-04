#include "STC15W4K.h"      // 包含 "STC15W4K.H"寄存器定义头文件
#include "uart_repeater.h" 
#include "uart4.h"

#define    TIMER4_RELOAD_VALUE 	(MAIN_Fosc /100)//1秒100次



volatile bit	tx4Busy = 0;
static enum ODD_EVEN uart4OddEven = NONE_ODD_EVEN;


void timer4_init(void)
{
	T4T3M &= ~0x40;//定时功能
	T4T3M &= ~0x20;//12 T模式
	T4H = (u8)((65536UL - TIMER4_RELOAD_VALUE/12) / 256);
	T4L = (u8)((65536UL - TIMER4_RELOAD_VALUE/12) % 256);
	
	T4T3M &= ~0x80;//定时器不允许运行
	IE2   &= ~0x40;//禁止中断
}



void timer4_irq(void) interrupt 20
{
	T4T3M &= ~0x80;//定时器不允许运行
	IE2   &= ~0x40;//禁止中断
	MUTEX_LOCK();
	uartRepeater.inUart2.recStat    = FRAME_RECEIVED;
	MUTEX_UNLOCK();
}





void uart4_init(u32 baud, enum  ODD_EVEN oddEven)
{
#if 0
	u16 t4Reload = 0;
	//baud = MAIN_Fosc/(U16_FULL_VALUE-t4Reload)/4
	t4Reload = U16_FULL_VALUE - MAIN_Fosc/(baud<<2);
	S4CON = 0x10;		//8位数据,可变波特率
	S4CON |= 0x40;		//串口4选择定时器4为波特率发生器
	T4T3M |= 0x20;		//定时器4时钟为Fosc,即1T
	T4L = (u8)(t4Reload);		//设定定时初值
	T4H = (u8)(t4Reload>>8);		//设定定时初值
	T4T3M |= 0x80;		//启动定时器4
#endif

	//S4CON = 0x10;		//8位数据,可变波特率
	S4CON &= ~0x40;		//串口4选择定时器2为波特率发生器
	
	if((ODD == oddEven) || (EVEN == oddEven))
	{
		S4CON  |= 0x80; 	//方式1，9位数据,可变波特率
		
		S4CON  |= 0x10;//允许接收
		uart4OddEven = oddEven;
	}
	else
	{	
		S4CON &= ~0x80;//方式0，8位数据,可变波特率
		
		S4CON |= 0x10;//允许接收
		uart4OddEven = NONE_ODD_EVEN;
	}
	
	UART4_INT_ENABLE();
	timer4_init();

}

void uart4_send_byte(u8 byte)
{
		


#if 0
	u8 oddbit = 0;
	oddbit = odd_even_detect(byte);

	
	if(ODD ==  uart4OddEven)
	{
		oddbit?S4TB8_CLR():S4TB8_SET();
	}
	else if(EVEN == uart4OddEven)
	{
		oddbit?S4TB8_SET():S4TB8_CLR();
	}
#endif
#if ENABLE_TIMOUT_SEND		
	u16 cnt = 0;
	while(tx4Busy && (cnt++<TICKS_OUT));
#else	
	while(tx4Busy);
#endif	

	if(ODD ==  uart4OddEven)
	{
		parityTable256[byte]?S4TB8_CLR():S4TB8_SET();
	}
	else if(EVEN == uart4OddEven)
	{
		parityTable256[byte]?S4TB8_SET():S4TB8_CLR();
	}
	
	S4BUF = byte;
	tx4Busy = 1;
}
void uart4_send(u8 *pbuf,u16 n)
{
	u16 i = 0;
	
	if(pbuf)
	{
		for(i=0; i<n; i++)
		{
			uart4_send_byte(pbuf[i]);
		}
	}
}
void uart4_irq(void) interrupt 18  // 串行口4中断函数
{ 
	u8 byte;

	//EA = 0;//关中断
	if (TI4)
	{
		CLR_TI4();             
		tx4Busy = 0;	 
	}
	if(RI4)
	{
		CLR_RI4();
		byte = S4BUF;
		//S4BUF = byte;
		T4T3M &= ~0x80;//stop timer4
		T4H = (u8)((65536UL - TIMER4_RELOAD_VALUE/12) / 256);
		T4L = (u8)((65536UL - TIMER4_RELOAD_VALUE/12) % 256);
		T4T3M |= 0x80;//定时器允许运行
		IE2   |= 0x40;//使能中断
		
		inuart_receive_isr(byte, &uartRepeater.outUart, &uartRepeater.inUart2);
	}
	//EA = 1;//开中断
}
