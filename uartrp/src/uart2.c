#include "STC15W4K.h"      // 包含 "STC15W4K.H"寄存器定义头文件
#include "uart_repeater.h" 
#include "uart2.h"
#include "config.h"



#define    TIMER1_RELOAD_VALUE 	(MAIN_Fosc /100)//1秒100次

volatile bit	tx2Busy = 0;

static enum ODD_EVEN uart2OddEven = NONE_ODD_EVEN;

void timer1_init(void)
{
	TMOD &= ~0x30;
	TMOD |= 0x10;;	//工作模式, 0: 16位自动重装, 1: 16位定时/计数, 2: 8位自动重装
	TMOD &= ~0x40;	//定时
	INT_CLKO &= ~0x01;	//不输出时钟

#if 0
	AUXR |=  0x40;	//1T mode
	TH1 = (u8)((65536UL - TIMER1_RELOAD_VALUE) / 256);
	TL1 = (u8)((65536UL - TIMER1_RELOAD_VALUE) % 256);
#else
	AUXR &=  ~0x40;	//12T mode
	TH1 = (u8)((65536UL - TIMER1_RELOAD_VALUE/12) / 256);
	TL1 = (u8)((65536UL - TIMER1_RELOAD_VALUE/12) % 256);
	//TR1 = 1;
	//ET1 = 1;
#endif	
	TR1 = 0;
	ET1 = 0;
	TF1 = 0;
}


void timer1_irq(void) interrupt  3
{
	TR1 = 0;
	ET1 = 0;
	TF1 = 0;
	MUTEX_LOCK();
	uartRepeater.inUart1.recStat    = FRAME_RECEIVED;
	MUTEX_UNLOCK();
	
}



void uart2_init(u32 baud, enum  ODD_EVEN oddEven)
{
	u16 t2Reload = 0;
#if 0
#ifdef  MAIN_Fosc
	#if   MAIN_Fosc == 22118400L	
	    // 下面代码设置定时器2
		T2H  = 0xFD;	// 波特率：9600 /22.1184MHZ,1T
		T2L  = 0xC0;	// 波特率：9600 /22.1184MHZ,1T
		AUXR = 0x14;    // 0001 0100，T2R=1启动T2运行，T2x12=1，定时器2按1T计数 
		// 下面代码设置定串口2
		S2CON = 0x10;      	  // 0001 0000 S2M0=0(最普遍的8位通信）,REN=1（允许接收）
		
	#elif  MAIN_Fosc == 11059200L
		S2CON = 0x50;		//8位数据,可变波特率
		AUXR |= 0x04;		//定时器2时钟为Fosc,即1T
		T2L = 0xE0;		//设定定时初值
		T2H = 0xFE;		//设定定时初值
		AUXR |= 0x10;		//启动定时器2
		
	#endif
#else
	#error "MAIN_Fosc not define"
#endif
#endif
	//baud = MAIN_Fosc/(U16_FULL_VALUE-t2Reload)/4
	t2Reload = U16_FULL_VALUE - MAIN_Fosc/(baud<<2);
  	//S2CON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x04;		//定时器2时钟为Fosc,即1T
	T2L = (u8)t2Reload;		//设定定时初值
	T2H = (u8)(t2Reload>>8);		//设定定时初值
	AUXR |= 0x10;		//启动定时器2

	if((ODD == oddEven) || (EVEN == oddEven))
	{
		S2CON  |= 0x80;		//方式1，9位数据,可变波特率

		S2CON |= 0x10;//允许接收
		uart2OddEven = oddEven;
	}
	else
	{
		S2CON &= ~0x80;//方式0，8位数据,可变波特率
		
		S2CON |= 0x10;//允许接收
		uart2OddEven = NONE_ODD_EVEN;
	}
	
	UART2_INT_ENABLE();

	timer1_init();
}


void uart2_send_byte(u8 byte)
{

	
	

#if  0
	u8 oddbit = 0;
	oddbit = odd_even_detect(byte);

	
	if(ODD ==  uart2OddEven)
	{
		oddbit?S2TB8_CLR():S2TB8_SET();
	}
	else if(EVEN == uart2OddEven)
	{
		oddbit?S2TB8_SET():S2TB8_CLR();
	}
#endif	
	
#if ENABLE_TIMOUT_SEND	
	u16 cnt = 0;
	while(tx2Busy && (cnt++<TICKS_OUT));
#else	
	while(tx2Busy);
#endif	

	if(ODD ==  uart2OddEven)
	{	
		parityTable256[byte]?S2TB8_CLR():S2TB8_SET();
	}
	else if(EVEN == uart2OddEven)
	{
		parityTable256[byte]?S2TB8_SET():S2TB8_CLR();
	}
	S2BUF = byte;
	tx2Busy = 1;
}


void uart2_send(u8 *pbuf,u16 n)
{
	u16 i = 0;
	
	if(pbuf)
	{
		for(i=0; i<n; i++)
		{
			uart2_send_byte(pbuf[i]);
		}
	}
}



void uart2_irq(void) interrupt 8  // 串行口2中断函数
{ 
	u8 byte;

	//EA = 0;//关中断
	if (TI2)
	{
		CLR_TI2();             
		tx2Busy = 0;	 
	}
	if(RI2)
	{
		CLR_RI2();
		byte = S2BUF;
#if  0		 
		S2BUF = byte;
#else
		TR1 = 0;
		TH1 = (u8)((65536UL - TIMER1_RELOAD_VALUE/12) / 256);
		TL1 = (u8)((65536UL - TIMER1_RELOAD_VALUE/12) % 256);
		TR1 = 1;
		ET1 = 1;
		inuart_receive_isr(byte, &uartRepeater.outUart, &uartRepeater.inUart1);
#endif	
	//EA = 1;//开中断
	}
}
