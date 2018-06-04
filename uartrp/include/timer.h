#ifndef __TIMER_H__
#define __TIMER_H__

#include "STC15W4K.h"      // 包含 "STC15W4K.H"寄存器定义头文件
#include "config.h"

#define ENABLE_TIMER0   0
#define ENABLE_TIMER1   0
#define ENABLE_TIMER2   0

#define TICKS_OUT       400//400-->2.17ms  when  MAIN_Fosc is 22118400L


#define TEST_TIMER  0

#if   TEST_TIMER
extern u32 seconds;
#endif



#if     ENABLE_TIMER0
extern void	timer0_init(void);
extern u32 get_tick_ms(void);

#endif


#if     ENABLE_TIMER1
extern void	Timer1_init(void);
#endif

#if     ENABLE_TIMER2
extern void	Timer2_init(void);
#endif





#endif

