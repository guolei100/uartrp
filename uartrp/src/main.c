#include "STC15W4K.h"      // 包含 "STC15W4K.H"寄存器定义头文件
#include "uart1.h"
#include "uart2.h"
#include "uart3.h"
#include "uart4.h"
#include "delay.h"
#include "timer.h"
#include "debug.h"
#include "uart_repeater.h"
#include "eeprom.h"
#include <stdio.h>
#include <string.h>



#define LED_ON  0
#define LED_OFF 1

sbit LED = P5^5;




#define D_WDT_FLAG			(1<<7)
#define D_EN_WDT			(1<<5)
#define D_CLR_WDT			(1<<4)	//auto clear
#define D_IDLE_WDT			(1<<3)	//WDT counter when Idle
#define D_WDT_SCALE_2		0
#define D_WDT_SCALE_4		1
#define D_WDT_SCALE_8		2		//T=393216*N/fo
#define D_WDT_SCALE_16		3
#define D_WDT_SCALE_32		4
#define D_WDT_SCALE_64		5
#define D_WDT_SCALE_128		6
#define D_WDT_SCALE_256		7


//unsigned char code rst[6]={0xe4,0xc0,0xe0,0xc0,0xe0,0x22};
//(*((void (*)())rst))();//复位


#if 0
void	GPIO_config(void)
{
	P5M0 = 0; 		//ÉèÖÃ×¼Ë«Ïò¿Ú
	P5M1 = 0;
}

void led_runnig(void)
{
		LED = LED_ON;
		delay_ms(250);
		delay_ms(250);
		delay_ms(250);
		delay_ms(250);
		
		LED = LED_OFF;
		delay_ms(250);
		delay_ms(250);
		delay_ms(250);
		delay_ms(250);
}

#endif
#define TEST_TYPE_LONG   0


#if TEST_TYPE_LONG
void test_printf(void)
{
	
	u32 u8type;
	u32 u16type;
	u32 u32type;
	u32 inttype;
	u32 longtype;
	u8 u8Data= 32;
	u16 u16Data = 1024;
	u32 u32Data = 100;

	u8type = sizeof(u8);
	u16type = sizeof(u16);
	u32type = sizeof(u32);
	inttype = sizeof(int);
	longtype = sizeof(long);
	
	DEBUG("\r\nu8 size:%ld\r\n",  u8type);
	DEBUG("u16 size:%ld\r\n", (u16type));
	DEBUG("int size:%ld\r\n", (inttype));
	DEBUG("u32 size:%ld\r\n",  (u32type));	
	DEBUG("long size:%ld\r\n", (longtype));

	//c51 printf
	DEBUG("u8Data:%bd\r\n",u8Data);
	DEBUG("u8Data:%bu\r\n",u8Data);
	DEBUG("u8Data:0x%bx\r\n",u8Data);

	DEBUG("u16Data:%hd\r\n",u16Data);
	DEBUG("u16Data:0x%hx\r\n",u16Data);
	
	DEBUG("u32Data:%ld\r\n",u32Data);

	
}
#endif


#if   TEST_TIMER
void test_timer(void)
{
	
	static u32 secs = 0;
		
	if(secs != seconds)
	{
		secs = seconds;
		DEBUG("min:%ld seconds:%ld\r\n",seconds/60,seconds%60);
	}
}
#endif	

void run_led(void)
{
	static u32 cnt = 0;

	if(5000==(cnt++))
	{
		LED = LED_ON;
		P32 = 0;
		
		P24 = 0;
		//P22 = 0;
		P21 = 0;

		P12 = 0;
		
		
	}
	else if(10000==(cnt++))
	{
		LED = LED_OFF;
		P32 = 1;

		P24 = 1;
		//P22 = 1;
		P21 = 1;

		P12 = 1;
		
		
		cnt = 0;
	}
				
}


u8 eepromWrBuf[12] = {0x99,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x55,0xaa};
u8 eepromRdBuf[12];

void test_eeprom(void)
{
	
	u8 i = 0;

	
	
#if 1
	eeprom_erase_sector(0x00);
	eeprom_write(0x00, eepromWrBuf, sizeof(eepromWrBuf));

	
	memset(eepromRdBuf,0,sizeof(eepromRdBuf));
	eeprom_read(0x00, eepromRdBuf, sizeof(eepromRdBuf));
	for(i=0; i<sizeof(eepromRdBuf);i++)
	{
		DEBUG("eepromRdBuf[%bd]=0x%bx\r\n",i,eepromRdBuf[i]);
	}
#endif	
}

void test_uart(void)
{
	while(1)
	{
		if(uart1RevLen>0)
		{
			uart1_send(uart1RevBuf,uart1RevLen);
			uart1RevLen = 0;
		}
		uart2_send(eepromWrBuf,12);
		uart4_send(eepromWrBuf,12);
		uart3_send(eepromWrBuf,12);
		delay_ms(250);
		delay_ms(250);
		delay_ms(250);
		delay_ms(250);
		
		delay_ms(250);
		delay_ms(250);
		delay_ms(250);
		delay_ms(250);
	}
}

#define  JIAO_XIAN    NONE_ODD_EVEN
void main()
{	
	//P24推挽输出
	P2M1 &= ~0x10;
	P2M0 |=  0x10;

	//P22推挽输出
	P2M1 &= ~0x04;
	P2M0 |=  0x04;

	//P21推挽输出
	P2M1 &= ~0x02;
	P2M0 |=  0x02;

	
	P24 = 0;
	P25 = 0;
	
	
	//timer0_init();
	//timer_init(get_tick_ms,0xffffffff);
	
	LED = LED_OFF;
	P32 = 0;
	
	//enable interrupt
	if(repeater_init() )
	{
		DEBUG("repeater_init fail\r\n");
	}
	else
	{
		DEBUG("repeater_init successfully\r\n");
	}
	
	//test_printf();
	//test_eeprom();

	
	
	//test_uart();
	while(1)
	{
		repeater_running();
		//run_led();
		//uart_transmit_dev();
		
		//timeout = (12*D_WDT_SCALE_16*32768)/MAIN_Fosc
		WDT_CONTR = (D_EN_WDT + D_CLR_WDT + D_WDT_SCALE_128);	//watchdog
		
	}
}  


