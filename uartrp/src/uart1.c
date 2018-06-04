#include "STC15W4K.h"      // 包含 "STC15W4K.H"寄存器定义头文件
#include "uart_repeater.h" 

#define	TIMER0_RELOAD_VALUE 	(MAIN_Fosc / 100)		//Timer 1 中断频率, 100次/秒


volatile bit	tx1_Busy = 0;

static enum ODD_EVEN uart1OddEven = NONE_ODD_EVEN;

u8  uart1RevBuf[REV_BUFF_LEN];
u16 uart1RevLen = 0;

void timer0_init(void)
{
		TR0 = 0;	//停止计数

	#if (TIMER0_RELOAD_VALUE < 64)	// 如果用户设置值不合适， 则不启动定时器
		#error "Timer0设置的中断过快!"

	#elif ((TIMER0_RELOAD_VALUE/12) < 65536UL)	// 如果用户设置值不合适， 则不启动定时器
		//ET0 = 1;	//允许中断
		TMOD &= ~0x03;
		TMOD |= 0x01;	//工作模式, 0: 16位自动重装, 1: 16位定时/计数, 2: 8位自动重装, 3: 16位自动重装, 不可屏蔽中断
	//	TMOD |=  0x04;	//对外计数或分频
		TMOD &= ~0x04;	//定时
	
		INT_CLKO &= ~0x01;	//不输出时钟

		#if (TIMER0_RELOAD_VALUE < 65536UL)
			AUXR |=  0x80;	//1T mode
			TH0 = (u8)((65536UL - TIMER0_RELOAD_VALUE) / 256);
			TL0 = (u8)((65536UL - TIMER0_RELOAD_VALUE) % 256);
		#else
			AUXR &= ~0x80;	//12T mode
			TH0 = (u8)((65536UL - TIMER0_RELOAD_VALUE/12) / 256);
			TL0 = (u8)((65536UL - TIMER0_RELOAD_VALUE/12) % 256);
		#endif

		//TR0 = 1;	//开始运行
		 ET0=0;
		 TR0=0;
		 TF0=0;
	#else
		#error "Timer0设置的中断过慢!"
	#endif
}




void timer0_irq(void) interrupt 1
{
	  ET0=0;
	  TR0=0;
	  TF0=0;
	 
}



void uart1_init(u32 baud, enum  ODD_EVEN oddEven)
{


#if 0

	    PCON &= 0x7F;		//波特率不倍速
		SCON  = 0x50;		//方式1，8位数据,可变波特率
		AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
		AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
		TMOD &= 0x0F;		//清除定时器1模式位
		TMOD |= 0x20;		//设定定时器1为8位自动重装方式
		
		//baud=((2^SMOD)/32)*MAIN_Fosc/(256-TH1) 
		TL1 = U8_FULL_VALUE-MAIN_Fosc/baud/32;
		TH1 = TL1;
		ET1   = 0;		    //禁止定时器1中断
		TR1   = 1;		    //启动定时器1
#endif
	
		PCON &= 0x7F;		//波特率不倍速
		AUXR |= 0x01;       //串口1选择定时器2为波特率发生器
		if((ODD == oddEven) || (EVEN == oddEven))
		{
			SCON  |= 0xc0;		//方式3，9位数据,可变波特率

			SCON  |= 0x10;//允许接收
			uart1OddEven = oddEven;
		}
		else
		{
			SCON &= ~0xc0;		
			SCON |= 0X40;//方式1，8位数据,可变波特率
			
			SCON |= 0x10;//允许接收
			uart1OddEven = NONE_ODD_EVEN;
		}

		ES    = 1;		    // 开串口1中断
		timer0_init();

} 

void uart1_send_byte(u8 byte)
{
	
	

	
	
#if ENABLE_TIMOUT_SEND	
	u16 cnt = 0;
	while(tx1_Busy && (cnt++<TICKS_OUT));
#else	
	while(tx1_Busy);
#endif	


	if(ODD ==  uart1OddEven)
	{
		TB8 = ~parityTable256[byte];
	}
	else if(EVEN == uart1OddEven)
	{
		TB8 = parityTable256[byte];
	}
	SBUF = byte;
	tx1_Busy = 1;
}
void uart1_send(u8 *pbuf,u16 n)
{
	u16 i = 0;
	
	if(pbuf)
	{
		for(i=0; i<n; i++)
		{
			uart1_send_byte(pbuf[i]);
		}
	}
}

//UART1 发送字符串
void UART1_SendString(char *s)
{
    while(*s)//检测字符串结束符
    {
        uart1_send_byte(*s++);//发送当前字符
    }
}

//重写putchar函数
char putchar(char c)
{
    uart1_send_byte(c);
    return c;
}

void uart1_irq(void) interrupt 4  // 串行口1中断函数
{
	u8 byte;
	
	//EA = 0;//关中断
	if(TI)
	{
		TI = 0;	
		tx1_Busy = 0; 		
	}
	if(RI)
	{
		RI = 0;
		byte = SBUF;
		if(uart1RevLen>=sizeof(uart1RevBuf))
		{
			uart1RevLen = 0;
		}
		uart1RevBuf[uart1RevLen++] = byte;
		
		//SBUF = byte;	   // 启动数据发送过程		

		TR0=0;
	#if (TIMER0_RELOAD_VALUE < 65536UL)
		TH0 = (u8)((65536UL - TIMER0_RELOAD_VALUE) / 256);
		TL0 = (u8)((65536UL - TIMER0_RELOAD_VALUE) % 256);
	#else
		TH0 = (u8)((65536UL - TIMER0_RELOAD_VALUE/12) / 256);
		TL0 = (u8)((65536UL - TIMER0_RELOAD_VALUE/12) % 256);
	#endif
		ET0=1;
	  	TR0=1;
		



	}
	//EA = 1;//开中断
}
