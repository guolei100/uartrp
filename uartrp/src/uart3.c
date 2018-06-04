#include "STC15W4K.h"      // 包含 "STC15W4K.H"寄存器定义头文件
#include "uart_repeater.h" 
#include "uart3.h"



#define    TIMER3_RELOAD_VALUE 	(MAIN_Fosc /100)//1秒100次


volatile bit	tx3Busy = 0;

static enum ODD_EVEN uart3OddEven = NONE_ODD_EVEN;

void timer3_init(void)
{
	T4T3M &= ~0x04;//定时功能
	T4T3M &= ~0x02;//12 T模式
	T3H = (u8)((65536UL - TIMER3_RELOAD_VALUE/12) / 256);
	T3L = (u8)((65536UL - TIMER3_RELOAD_VALUE/12) % 256);
	
	T4T3M &= ~0x08;//定时器不允许运行
	IE2   &= ~0x20;//禁止中断
}


void timer3_irq(void) interrupt 19
{
	T4T3M &= ~0x08;//定时器不允许运行
	IE2   &= ~0x20;//禁止中断
	MUTEX_LOCK();
	uartRepeater.outUart.uart.recStat    = FRAME_RECEIVED;
	MUTEX_UNLOCK();
	P32 = 1;

	
}



void uart3_init( enum  ODD_EVEN oddEven)
{
#if 0
	u16 t3Reload = 0;
	//baud = MAIN_Fosc/(U16_FULL_VALUE-t2Reload)/4
	t3Reload = U16_FULL_VALUE - MAIN_Fosc/(baud<<2);

	S3CON = 0x10;		//8位数据,可变波特率
	S3CON |= 0x40;		//串口3选择定时器3为波特率发生器
	T4T3M |= 0x02;		//定时器3时钟为Fosc,即1T
	T3L = (u8)t3Reload;		//设定定时初值
	T3H = (u8)(t3Reload>>8);		//设定定时初值
	T4T3M |= 0x08;		//启动定时器3
#endif
	//S3CON = 0x10;		//8位数据,可变波特率
	S3CON &= ~0x40;     //串口3选择定时器2为波特率发生器

	if((ODD == oddEven) || (EVEN == oddEven) || (MARK == oddEven) || (SPACE == oddEven))
	{
		S3CON  |= 0x80;		//方式1，9位数据,可变波特率
		
		S3CON  |= 0x10;//允许接收
		uart3OddEven = oddEven;
	}
	else
	{	
		S3CON &= ~0X80;//方式0，8位数据,可变波特率
		
		S3CON |= 0x10;//允许接收
		uart3OddEven = NONE_ODD_EVEN;
	}
	
	UART3_INT_ENABLE();	    	  // 开串口3中断

	timer3_init();
}


void uart3_send_byte(u8 byte)
{

		
	
#if   0
	u8 oddbit = 0;
	oddbit = odd_even_detect(byte);

	
	if(ODD ==  uart3OddEven)
	{
		oddbit?S3TB8_CLR():S3TB8_SET();
	}
	else if(EVEN == uart3OddEven)
	{
		oddbit?S3TB8_SET():S3TB8_CLR();
	}
#endif
	
#if ENABLE_TIMOUT_SEND	
	u16 cnt = 0;
	while(tx3Busy && (cnt++<TICKS_OUT));
#else	
	while(tx3Busy);
#endif	
	if(ODD ==  uart3OddEven)
	{
		parityTable256[byte]?S3TB8_CLR():S3TB8_SET();
	}
	else if(EVEN == uart3OddEven)
	{
		parityTable256[byte]?S3TB8_SET():S3TB8_CLR();
	}
	else if(MARK == uart3OddEven)
	{
		S3TB8_SET();
	}
	else if(SPACE == uart3OddEven)
	{
		S3TB8_CLR();
	}
	S3BUF = byte;
	tx3Busy = 1;
}
void uart3_send(u8 *pbuf,u16 n)
{
	u16 i = 0;
	
	if(pbuf)
	{
		for(i=0; i<n; i++)
		{
			uart3_send_byte(pbuf[i]);
		}
	}
}

  

void uart3_irq(void) interrupt 17  // 串行口3中断函数
{ 
	u8 byte;

	//EA = 0;//关中断
	if (TI3)
	{
		CLR_TI3();             
		tx3Busy = 0;	 
	}
	if(RI3)
	{
		P32 = 0;
		CLR_RI3();
		byte = S3BUF;		
		//S3BUF=byte;
		T4T3M &= ~0x08;//stop timer3
		T3H = (u8)((65536UL - TIMER3_RELOAD_VALUE/12) / 256);
		T3L = (u8)((65536UL - TIMER3_RELOAD_VALUE/12) % 256);
		T4T3M |= 0x08;//定时器允许运行
		IE2   |= 0x20;
		
		outuart_receive_isr(byte, &uartRepeater);
	}
	//EA = 1;//开中断
}
