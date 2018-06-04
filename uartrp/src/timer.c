

/*************	功能说明	**************

本程序演示3个定时器的使用, 本例程均使用16位自动重装.

下载时, 选择时钟 24MHZ (用户可自行修改频率).

定时器0做16位自动重装, 中断频率为1000HZ，中断函数从P1.7取反输出500HZ方波信号.

定时器1做16位自动重装, 中断频率为2000HZ，中断函数从P1.6取反输出1000HZ方波信号.

定时器2做16位自动重装, 中断频率为3000HZ，中断函数从P4.7取反输出1500HZ方波信号.

******************************************/




#include	"STC15W4K.H"
#include    "timer.h"
#include    "config.h"

#include    "timer.h"
#include    "universal_timer.h"
#include    "debug.h"


#ifdef  MAIN_Fosc
#define	Timer0_Reload	(MAIN_Fosc / 1000)		//Timer 0 中断频率, 1000次/秒
#define	Timer1_Reload	(MAIN_Fosc / 2000)		//Timer 1 中断频率, 2000次/秒
#define	Timer2_Reload	(MAIN_Fosc / 3000)		//Timer 2 中断频率, 3000次/秒

#else
#error "MAIN_Fosc not define"
#endif







//========================================================================
// 函数: void main(void)
// 描述: 主函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2015-1-12
//========================================================================
#if  0
void timer_init(void)
{
	P0M1 = 0;	P0M0 = 0;	//设置为准双向口
	P1M1 = 0;	P1M0 = 0;	//设置为准双向口
	P2M1 = 0;	P2M0 = 0;	//设置为准双向口
	P3M1 = 0;	P3M0 = 0;	//设置为准双向口
	P4M1 = 0;	P4M0 = 0;	//设置为准双向口
	P5M1 = 0;	P5M0 = 0;	//设置为准双向口
	P6M1 = 0;	P6M0 = 0;	//设置为准双向口
	P7M1 = 0;	P7M0 = 0;	//设置为准双向口

	EA = 1;		//打开总中断

#if     ENABLE_TIMER0		
	Timer0_init();
#endif
#if     ENABLE_TIMER1	
	Timer1_init();
#endif

#if     ENABLE_TIMER2
	Timer2_init();
#endif
	while (1)
	{

	}
}
#endif

//========================================================================
// 函数: void	Timer0_init(void)
// 描述: timer0初始化函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2015-1-12
//========================================================================
#if     ENABLE_TIMER0	
void	timer0_init(void)
{
		TR0 = 0;	//停止计数

	#if (Timer0_Reload < 64)	// 如果用户设置值不合适， 则不启动定时器
		#error "Timer0设置的中断过快!"

	#elif ((Timer0_Reload/12) < 65536UL)	// 如果用户设置值不合适， 则不启动定时器
		ET0 = 1;	//允许中断
	//	PT0 = 1;	//高优先级中断
		TMOD &= ~0x03;
		TMOD |= 0;	//工作模式, 0: 16位自动重装, 1: 16位定时/计数, 2: 8位自动重装, 3: 16位自动重装, 不可屏蔽中断
	//	TMOD |=  0x04;	//对外计数或分频
		TMOD &= ~0x04;	//定时
	//	INT_CLKO |=  0x01;	//输出时钟
		INT_CLKO &= ~0x01;	//不输出时钟

		#if (Timer0_Reload < 65536UL)
			AUXR |=  0x80;	//1T mode
			TH0 = (u8)((65536UL - Timer0_Reload) / 256);
			TL0 = (u8)((65536UL - Timer0_Reload) % 256);
		#else
			AUXR &= ~0x80;	//12T mode
			TH0 = (u8)((65536UL - Timer0_Reload/12) / 256);
			TL0 = (u8)((65536UL - Timer0_Reload/12) % 256);
		#endif

		TR0 = 1;	//开始运行

	#else
		#error "Timer0设置的中断过慢!"
	#endif
}
#endif
//========================================================================
// 函数: void	Timer1_init(void)
// 描述: timer1初始化函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2015-1-12
//========================================================================
#if     ENABLE_TIMER1	

void	Timer1_init(void)
{
		TR1 = 0;	//停止计数

	#if (Timer1_Reload < 64)	// 如果用户设置值不合适， 则不启动定时器
		#error "Timer1设置的中断过快!"

	#elif ((Timer1_Reload/12) < 65536UL)	// 如果用户设置值不合适， 则不启动定时器
		ET1 = 1;	//允许中断
	//	PT1 = 1;	//高优先级中断
		TMOD &= ~0x30;
		TMOD |= (0 << 4);	//工作模式, 0: 16位自动重装, 1: 16位定时/计数, 2: 8位自动重装
	//	TMOD |=  0x40;	//对外计数或分频
		TMOD &= ~0x40;	//定时
	//	INT_CLKO |=  0x02;	//输出时钟
		INT_CLKO &= ~0x02;	//不输出时钟

		#if (Timer1_Reload < 65536UL)
			AUXR |=  0x40;	//1T mode
			TH1 = (u8)((65536UL - Timer1_Reload) / 256);
			TL1 = (u8)((65536UL - Timer1_Reload) % 256);
		#else
			AUXR &= ~0x40;	//12T mode
			TH1 = (u8)((65536UL - Timer1_Reload/12) / 256);
			TL1 = (u8)((65536UL - Timer1_Reload/12) % 256);
		#endif

		TR1 = 1;	//开始运行

	#else
		#error "Timer1设置的中断过慢!"
	#endif
}
#endif
//========================================================================
// 函数: void	Timer2_init(void)
// 描述: timer2初始化函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2015-1-12
//========================================================================
#if     ENABLE_TIMER2	

void	Timer2_init(void)
{
		AUXR &= ~0x1c;		//停止计数, 定时模式, 12T模式

	#if (Timer2_Reload < 64)	// 如果用户设置值不合适， 则不启动定时器
		#error "Timer2设置的中断过快!"

	#elif ((Timer2_Reload/12) < 65536UL)	// 如果用户设置值不合适， 则不启动定时器
	//	IE2  &= ~(1<<2);	//禁止中断
		IE2  |=  (1<<2);	//允许中断
	//	INT_CLKO |=  0x04;	//输出时钟
		INT_CLKO &= ~0x04;	//不输出时钟

	//	AUXR |=  (1<<3);	//对外计数或分频
	//	INT_CLKO |=  0x02;	//输出时钟
		INT_CLKO &= ~0x02;	//不输出时钟

		#if (Timer1_Reload < 65536UL)
			AUXR |=  (1<<2);	//1T mode
			T2H = (u8)((65536UL - Timer2_Reload) / 256);
			T2L = (u8)((65536UL - Timer2_Reload) % 256);
		#else
			T2H = (u8)((65536UL - Timer2_Reload/12) / 256);
			T2L = (u8)((65536UL - Timer2_Reload/12) % 256);
		#endif

			AUXR |=  (1<<4);	//开始运行

	#else
		#error "Timer2设置的中断过慢!"
	#endif
}
#endif

//========================================================================
// 函数: void timer0_int (void) interrupt TIMER0_VECTOR
// 描述:  timer0中断函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2015-1-12
//========================================================================


#if  ENABLE_TIMER0	
volatile u32 timer0Cnt = 0;

#if     TEST_TIMER
u32 seconds = 0;
#endif

void timer0_int (void) interrupt 1
{

	timer0Cnt++;
	process_timer();
   
#if   TEST_TIMER 
	if(0 == timer0Cnt%1000)
	{
		P26 = ~P26;
		seconds++;
	}
#endif 
   
}


#endif



//========================================================================
// 函数: void timer1_int (void) interrupt TIMER1_VECTOR
// 描述:  timer1中断函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2015-1-12
//========================================================================
#if     ENABLE_TIMER1	

void timer1_int (void) interrupt TIMER1_VECTOR
{
   P16 = ~P16;

}
#endif
//========================================================================
// 函数: void timer2_int (void) interrupt TIMER2_VECTOR
// 描述:  timer2中断函数.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2015-1-12
//========================================================================
#if     ENABLE_TIMER2	

void timer2_int (void) interrupt TIMER2_VECTOR
{
	P47 = ~P47;
}
#endif


#if     ENABLE_TIMER0

u32 get_tick_ms(void)
{
	return timer0Cnt;
}


#endif





