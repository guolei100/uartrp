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


#define  U16_FULL_VALUE   65536UL
#define  U8_FULL_VALUE    256

#define  ENABLE_TIMOUT_SEND  0



typedef   char           s8;
typedef   unsigned char  u8;
typedef   short          s16;
typedef   unsigned short u16;
typedef   long           s32;
typedef   unsigned long  u32; 


#endif
