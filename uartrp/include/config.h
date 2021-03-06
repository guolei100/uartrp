#ifndef		__CONFIG_H__
#define		__CONFIG_H__

#include	"STC15W4K.H"


/*********************************************************/

#define MAIN_Fosc		22118400UL	//定义主时钟

//#define MAIN_Fosc		12000000UL	//定义主时钟

//#define MAIN_Fosc		11059200UL	//定义主时钟

//#define MAIN_Fosc		 5529600UL	//定义主时钟

//#define MAIN_Fosc		24000000UL	//定义主时钟


/*********************************************************/


#define  U16_FULL_VALUE       65536UL
#define  U8_FULL_VALUE        256

#define  ENABLE_TIMOUT_SEND   0
 
#define  SAVE_MARK            0x55aa
#define  PARAM_SAVE_ADDR      0x0000


#define  DEFAULT_BAUD         115200

/*************test MACRO****************/

#define  TEST_UART            0//1: test uart
#define  SET_BAUD_DEFULT      1//1: baud use DEFAULT_BAUD dont't read from eeprom
#define  DEBUG_FRAM_TRACK     0// 1: print all uarts recive and send frames

/*************test MACRO****************/


#define  TIMER_RELOAD_VALUE   (MAIN_Fosc / 200)		//       瓒呮椂:5ms


//#define  TIMER_RELOAD_VALUE  (MAIN_Fosc / 500)		//500娆�/绉�       瓒呮椂:2ms
//#define  TIMER_RELOAD_VALUE  (MAIN_Fosc / 200)		//200娆�/绉�       瓒呮椂:5ms
//#define  TIMER_RELOAD_VALUE  (MAIN_Fosc / 100)		//100娆�/绉�       瓒呮椂:10ms
//#define  TIMER_RELOAD_VALUE  (MAIN_Fosc / 50)         //50娆�/绉�        瓒呮椂:20ms 
//#define  TIMER0_RELOAD_VALUE  (MAIN_Fosc / 30)         //            瓒呮椂:33ms 

typedef   char           s8;
typedef   unsigned char  u8;
typedef   short          s16;
typedef   unsigned short u16;
typedef   long           s32;
typedef   unsigned long  u32; 


#endif
