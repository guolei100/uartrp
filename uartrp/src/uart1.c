#include "STC15W4K.h"      // 包含 "STC15W4K.H"寄存器定义头文件
#include "uart_repeater.h" 



#define  TIMER0_RELOAD_VALUE  (MAIN_Fosc / 200)         //     超时:5ms 

volatile bit	tx1_Busy = 0;

static enum ODD_EVEN uart1OddEven = NONE_ODD_EVEN;
static u8 CntOf5ms = 0;

int (*pTmr1Out_callback)(void );


void timer0_init(void)
{
		pTmr1Out_callback = NULL;
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
	if(CntOf5ms)
	{
		CntOf5ms--;
		
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
	else
	{
		MUTEX_LOCK();
		if(pTmr1Out_callback)
		{
			pTmr1Out_callback();
			pTmr1Out_callback = NULL;
		}
		else
		{
			uartRepeater.proUart.recStat = FRAME_RECEIVED;
#if  DEBUG_FRAM_TRACK
			uartRepeater.proUart.frmRevCnt++;
#endif
		}
		MUTEX_UNLOCK();
	}
	 
}



void uart1_init( enum  ODD_EVEN oddEven)
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
		if((ODD == oddEven) || (EVEN == oddEven) || (MARK == oddEven) || (SPACE == oddEven))
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
	else if(MARK == uart1OddEven)
	{
		TB8 = 1;
	}
	else if(SPACE == uart1OddEven)
	{
		TB8 = 0;
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



void start_timer1(u8 countOf5ms,int (*pcallback)(void ))
{
		CntOf5ms = countOf5ms;
		pTmr1Out_callback = pcallback;
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
void stop_timer1(void)
{
	
		ET0=0;
		TR0=0;
		TF0=0;
	#if (TIMER0_RELOAD_VALUE < 65536UL)
		TH0 = (u8)((65536UL - TIMER0_RELOAD_VALUE) / 256);
		TL0 = (u8)((65536UL - TIMER0_RELOAD_VALUE) % 256);
	#else
		TH0 = (u8)((65536UL - TIMER0_RELOAD_VALUE/12) / 256);
		TL0 = (u8)((65536UL - TIMER0_RELOAD_VALUE/12) % 256);
	#endif
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
		uartRepeater.mode = SET_UP;
#if TEST_UART		
		SBUF = byte;	   // 启动数据发送过程		
#else		
		start_timer1(2,NULL);
		prouart_isr(byte,&uartRepeater.proUart);
#endif	
	
	}
	//EA = 1;//开中断
}
