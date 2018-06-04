#ifndef _UARTR_REPEATER_CFG_H_
#define _UARTR_REPEATER_CFG_H_

#include "universal_timer.h"
#include "timer.h"


#define   ENABLE_UART1       1
#define   ENABLE_UART2       1
#define   ENABLE_UART3       1
#define   ENABLE_UART4       1





#define   UART1_BUF_LEN      400
#define   UART2_BUF_LEN      400
#define   UART3_BUF_LEN      400
#define   UART4_BUF_LEN      400


#define   REV_BUFF_LEN       512

#define   A_RS485_SEND_ENABLE()   //P24 = 1
#define   A_RS485_SEND_DISABLE()  //P24 = 0

#define   B_RS485_SEND_ENABLE()   //P25 = 1
#define   B_RS485_SEND_DISABLE() // P25 = 0

#define   MUTEX_LOCK()        EA = 0
#define   MUTEX_UNLOCK()      EA = 1



typedef void (*send)(u8 *dat, u16 len);


enum  SEND_STATE{
	NO_SEND    = 0,
	SENDING    = 1,
	WAIT_SEND  = 2
};

enum  RECEIVE_STATE{
	NO_RECEIVE   = 0,
	FRAME_RECEIVING    = 1,
	FRAME_RECEIVED     = 2
};

enum  UART_PORT{
	UART1_PORT     = 0x01,
	UART2_PORT     = 0x02,
	UART3_PORT     = 0x03,
	UART4_PORT     = 0x04,
	NONE_UART      = 0xff
};

enum  ODD_EVEN{
	NONE_ODD_EVEN = 0,
	ODD           = 1, //奇校验
	EVEN          = 2, //偶校验
	MARK          = 3, //正校验
	SPACE         = 4  //负校验
};
enum  MODE_SET{
	NORMAL_COM  = 0,
	SET_UP      = 1//设置模式
};

typedef struct Uart_Baud
{
	enum UART_PORT   uartPort;
	s32              baud;
	u8               dataLen;
	enum  ODD_EVEN   oddEven;
	u8               stopBits;
	u16              saveMark;
	u32              sum;
}UART_BAUD;



typedef struct Uart_Way
{
	enum SEND_STATE      sendStat;//转发状态
	enum RECEIVE_STATE   recStat;
	UART_BAUD  baud;
	TIMER_TABLE* pTimer;
	send   send_fun; 
	u8  revBuf[REV_BUFF_LEN];
	u16 revLen;
	u16 revIndex; 
	u16 sendLen;//转发长度
	u16 sendIndex; 
}UART_WAY;


typedef struct Out_Uart
{
	UART_WAY        uart;
	TIMER_TABLE*    presTimer;
	enum UART_PORT  conUart;//收到的数据应该发给哪个串口 	
	
}OUT_UART;




typedef struct Uart_Repeater
{
	OUT_UART        outUart;
	UART_WAY        inUart1;//wifi
	UART_WAY        inUart2;
	UART_WAY        proUart;//uart1
	enum MODE_SET   mode; 
	u8             wifiCommStep;
}UART_RETPEATER;


extern const u8 parityTable256[256];

extern UART_RETPEATER uartRepeater;

extern u8 odd_even_detect(u8 byte);
extern void outuart_receive_isr(u8 recByte, UART_RETPEATER *prepeater);
extern void inuart_receive_isr(u8 recByte, OUT_UART *pOutUart, UART_WAY *pInUart);
extern void prouart_isr(u8 recByte, UART_WAY *pProUart);
extern void repeater_running(void);
extern s8 repeater_init(void); 

#endif

