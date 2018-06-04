#include "uart_repeater.h" 
#include "uart1.h"
#include "uart2.h"
#include "uart3.h"
#include "uart4.h"
#include "debug.h"

/************************************************************
                                     terminal A   uartRepeater.outUart
                        _________________________
                       |             | outUart   |------>uart3
                       |             |___________|
                       |___                      |
                       |i|                       |
                       |n|     uartRepeater      |
                       |U|                       |
            wifi model |a|                       |
                       |r|                       |
            uart2<-----|t|            ___________|
uartRepeater.inUart1   |1|           | inUart2   |
                       |_|___________|___________|

                                       terminal B------>uart4 
                                       uartRepeater.inUart2

************************************************************/


#if 0
const u8 ParityTable16[16] = {0,//0000
                              1,//0001
                              1,//0010
                              0,//0011
                              1,//0100
                              0,//0101
                              0,//0110
                              1,//0111
                              1,//1000
                              0,//1001
                              0,//1010
                              1,//1011
                              0,//1100
                              1,//1101
                              1,//1110
                              0 //1111
}
#endif

const u8 parityTable256[256] = {
0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 
};


UART_RETPEATER uartRepeater;



u8 odd_even_detect(u8 byte)
{
	u8 j=0;
	u8 i=0;
	u8 z=0;
	
	for(i=0;i<8;i++)
	{
		z=((byte>>i)&0x01);
		if(z)
		{
			j++;
		}
	}
	if(j&0x01)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int outuart_response_timer_out_callback(void *pArg)
{
	UART_RETPEATER *prepeater;

	if(NULL == pArg)
	{
		return -1;
	}
	
	prepeater = (UART_RETPEATER *)pArg;
	MUTEX_LOCK();
	prepeater->outUart.conUart            = NONE_UART;
	prepeater->outUart.uart.sendStat      = NO_SEND;//outUart only have NO_SEND and SENDING stat
	prepeater->outUart.uart.recStat       = NO_RECEIVE;
	prepeater->outUart.uart.baud.uartPort = UART3_PORT;
	prepeater->outUart.uart.revLen        = 0;
	prepeater->outUart.uart.sendLen       = 0;
	prepeater->outUart.uart.revIndex      = 0;
	prepeater->outUart.uart.sendIndex     = 0;
	MUTEX_UNLOCK();
	
	return 0;
}


int outUart_timer_callback(void *pArg)
{
	UART_RETPEATER *prepeater;

	if(NULL == pArg)
	{
		return -1;
	}
	
	prepeater = (UART_RETPEATER *)pArg;
	MUTEX_LOCK();
	prepeater->outUart.uart.recStat  = FRAME_RECEIVED;
	MUTEX_UNLOCK();
	
	return 0;
}
int inUart_timer_callback(void *pArg)
{
	UART_WAY *pInUart;

	if(NULL == pArg)
	{
		return -1;
	}
	pInUart = (UART_WAY *)pArg;
	MUTEX_LOCK();
	pInUart->recStat           = FRAME_RECEIVED;
	MUTEX_UNLOCK();
	
	
	return 0;
	
}
extern void repeater_stat_init(void);
void outuart_receive_isr(u8 recByte, UART_RETPEATER *prepeater)
{
	if( NULL == prepeater )
	{
		return ;
	}
	
	if( (prepeater->outUart.conUart == prepeater->inUart1.baud.uartPort) ||\
		(prepeater->outUart.conUart == prepeater->inUart2.baud.uartPort) )
	{
		//stop_timer(uartRepeater.outUart.presTimer);
		if(prepeater->outUart.uart.revIndex >= sizeof(prepeater->outUart.uart.revBuf) )
		{
			prepeater->outUart.uart.revIndex = 0;
		}
		prepeater->outUart.uart.revBuf[prepeater->outUart.uart.revIndex] = recByte;
		prepeater->outUart.uart.revIndex++;
		prepeater->outUart.uart.revLen++;
		MUTEX_LOCK();
		prepeater->outUart.uart.recStat = FRAME_RECEIVING;
		MUTEX_UNLOCK();
	}
	else
	{
		//repeater_stat_init();//收到不明数据初始化状态?
		return ;//discard 
	}

	//start_timer(prepeater->outUart.uart.pTimer);
	
	//stop_timer(prepeater->outUart.presTimer);
}


void inuart_receive_isr(u8 recByte, OUT_UART *pOutUart, UART_WAY *pInUart)
{
	if( (NULL == pInUart) || (NULL == pOutUart) )
	{
		return ;
	}
	MUTEX_LOCK();
	//if( NO_SEND == pOutUart->uart.sendStat )
	if( (NONE_UART == pOutUart->conUart) || (pOutUart->conUart == pInUart->baud.uartPort) )
	{
		
		pOutUart->uart.sendStat = SENDING;
		pOutUart->conUart = pInUart->baud.uartPort;
	}
	
	if(pInUart->revIndex >= sizeof(pInUart->revBuf) )
	{
		pInUart->revIndex = 0;
	}
	pInUart->revBuf[pInUart->revIndex] = recByte;
	pInUart->revIndex++;
	pInUart->revLen++;
	pInUart->recStat = FRAME_RECEIVING;
	
	MUTEX_UNLOCK();
	//start_timer(pInUart->pTimer);
	
}



static u8 from_uart_to_another_uart(UART_WAY *pFrom,UART_WAY *pToUart)
{
	u8 sendEnd = 0;
		
	if((NULL == pFrom) || (NULL == pToUart) )
	{
		return sendEnd;
	}
#if 0	
	if(FRAME_RECEIVING == pToUart->recStat)
	{
		//正在接收是不能发送的
		return sendEnd;
	}
#endif	
	if(pToUart->sendLen < pFrom->revLen)
	{
		if(pToUart->sendIndex >= sizeof(pFrom->revBuf))
		{
			pToUart->sendIndex = 0;
		}
		if(pToUart->send_fun)
		{
			pToUart->send_fun(&pFrom->revBuf[pToUart->sendIndex],1);
			if(pToUart->send_fun == uart4_send)
			{
				//uart1_send(&pFrom->revBuf[pToUart->sendIndex],1);
 			}
		}
		else
		{
			DEBUG("port:%bd send fun is NULL\r\n",pToUart->baud.uartPort);
		}
		pToUart->sendIndex++;
		pToUart->sendLen++;
		pToUart->sendStat  = SENDING;
	}
	if(NO_SEND == pToUart->sendStat)
	{//已发送完
		return sendEnd;
	}
	if( (FRAME_RECEIVED == pFrom->recStat) && (pFrom->revLen == pToUart->sendLen) )//转发完一帧
	{
		sendEnd = 1;//来自pInUart串口的数据已全部从pOutUart转发出去
	}
	return sendEnd;
	
	
}




//outUart  ----->  inUart1 or inUart2
static u8 send_to_in_uart(UART_WAY *pInUart,UART_WAY *pOutUart)
{
	u8 sndEnd = 0;
	
	if((NULL == pInUart) || (NULL == pOutUart) )
	{
		return sndEnd;
	}
	sndEnd = from_uart_to_another_uart(pOutUart,pInUart);
	if(sndEnd)
	{
		//DEBUG("s3:%bd r2:%bd\r\n",pInUart->sendIndex,pOutUart->revLen);
MUTEX_LOCK();		
		pOutUart->revIndex  = 0;
		pOutUart->revLen    = 0;
		pOutUart->recStat   = NO_RECEIVE;
MUTEX_UNLOCK();	
		pInUart->sendLen   = 0;
		pInUart->sendIndex = 0;
		pInUart->sendStat  = NO_SEND;
		
	
	}
	return sndEnd;
}




//inUart1 or inUart2  ----->  outUart  
static u8 send_to_out_uart(UART_WAY *pInUart,UART_WAY *pOutUart)
{
	u8 sendEnd = 0;


	sendEnd =  from_uart_to_another_uart(pInUart, pOutUart);
	if(sendEnd)
	{
		/*应当收到pOutUart回应，并将此回应转发给pInUart完成后此连接才断开*/	
#if 0
		if(uartRepeater.outUart.conUart == UART4_PORT)
		{
			DEBUG("rev:%hd send:%hd\r\n",pInUart->revLen,pOutUart->sendLen);
		}
	
		if(uartRepeater.outUart.conUart == UART2_PORT)
		{
			DEBUG("rev:%hd send:%hd\r\n",pInUart->revLen,pOutUart->sendLen);
		}
#endif		
MUTEX_LOCK();		
		pInUart->revIndex   = 0;
		pInUart->revLen     = 0;
		pInUart->recStat    = NO_RECEIVE;
MUTEX_UNLOCK();	
		pOutUart->sendIndex = 0;
		pOutUart->sendLen   = 0;
		pOutUart->sendStat  = NO_SEND;
		
	}
	return sendEnd;
	
	
}

void reponse_inuart( UART_RETPEATER *prepeater)
{
	u8 sendEnd = 0;
	
	if( NULL == prepeater )
	{
		return ;
	}
	if(prepeater->outUart.conUart == prepeater->inUart1.baud.uartPort)
	{
		sendEnd = send_to_in_uart(&prepeater->inUart1,&prepeater->outUart.uart);
		if(sendEnd)
		{
			prepeater->outUart.uart.sendLen   = 0;
			prepeater->outUart.uart.sendIndex = 0;
			if(prepeater->inUart2.revLen > 0)
			{
MUTEX_LOCK();			
				prepeater->outUart.conUart        = prepeater->inUart2.baud.uartPort;
				prepeater->outUart.uart.sendStat  = SENDING;
MUTEX_UNLOCK();				
				
			}
			else
			{
MUTEX_LOCK();			
				prepeater->outUart.conUart        = NONE_UART;
				prepeater->outUart.uart.sendStat  = NO_SEND;
MUTEX_UNLOCK();				
			}
		}
	}
	else if(prepeater->outUart.conUart == prepeater->inUart2.baud.uartPort)
	{
		sendEnd = send_to_in_uart(&prepeater->inUart2, &prepeater->outUart.uart);
		if(sendEnd)
		{
			prepeater->outUart.uart.sendLen   = 0;
			prepeater->outUart.uart.sendIndex = 0;
			if(prepeater->inUart1.revLen > 0)
			{
MUTEX_LOCK();				
				prepeater->outUart.uart.sendStat  = SENDING;
				prepeater->outUart.conUart        = prepeater->inUart1.baud.uartPort;
MUTEX_UNLOCK();

			}
			else
			{
MUTEX_LOCK();			
				prepeater->outUart.uart.sendStat  = NO_SEND;
				prepeater->outUart.conUart        = NONE_UART;
MUTEX_UNLOCK();				
			}
		}
	}
	else
	{
		
	}
}


void repeater_running(void)
{
	u8 sndEnd = 0;

	/******************** data from inUart1 or inUart2 to  outuart*******/
	if(uartRepeater.outUart.conUart == uartRepeater.inUart1.baud.uartPort)
	{
		if(SENDING == uartRepeater.outUart.uart.sendStat)
		{
			
			A_RS485_SEND_ENABLE();
			sndEnd = send_to_out_uart(&uartRepeater.inUart1,&uartRepeater.outUart.uart);
			if(sndEnd)
			{
				A_RS485_SEND_DISABLE();
				//加个超时机制，以判断outUart在规定时间人没有应答
				//start_timer(uartRepeater.outUart.presTimer);
			}
		}
		
	}
	else if(uartRepeater.outUart.conUart == uartRepeater.inUart2.baud.uartPort)
	{
		if(SENDING == uartRepeater.outUart.uart.sendStat)
		{
			//B_RS485_SEND_ENABLE();
			sndEnd = send_to_out_uart(&uartRepeater.inUart2,&uartRepeater.outUart.uart);
			if(sndEnd)
			{
				//B_RS485_SEND_DISABLE();
				//加个超时机制，以判断outUart在规定时间人没有应答
				//start_timer(uartRepeater.outUart.presTimer);
			}
		}
		
		
	}
	
	/******************** data from inUart1 or inUart2 to  outuart*******/
	
	reponse_inuart(&uartRepeater);

	
	
}

void repeater_stat_init(void)
{
	uartRepeater.outUart.conUart            = NONE_UART;
	uartRepeater.outUart.uart.sendStat      = NO_SEND;//outUart only have NO_SEND and SENDING stat
	uartRepeater.outUart.uart.recStat       = NO_RECEIVE;
	uartRepeater.outUart.uart.baud.uartPort = UART3_PORT;
	uartRepeater.outUart.uart.revLen        = 0;
	uartRepeater.outUart.uart.sendLen       = 0;
	uartRepeater.outUart.uart.revIndex      = 0;
	uartRepeater.outUart.uart.sendIndex     = 0;

	//inUart1 init
	uartRepeater.inUart1.sendStat           = NO_SEND;
	uartRepeater.inUart1.recStat            = NO_RECEIVE;
	uartRepeater.inUart1.baud.uartPort      = UART2_PORT;
	uartRepeater.inUart1.revLen             = 0;
	uartRepeater.inUart1.sendLen            = 0;
	uartRepeater.inUart1.revIndex           = 0;
	uartRepeater.inUart1.sendIndex          = 0;

	//inUart2 init
	uartRepeater.inUart2.sendStat           = NO_SEND;
	uartRepeater.inUart2.recStat            = NO_RECEIVE;
	uartRepeater.inUart2.baud.uartPort      = UART4_PORT;
	uartRepeater.inUart2.revLen             = 0;
	uartRepeater.inUart2.sendLen            = 0;
	uartRepeater.inUart2.revIndex           = 0;
	uartRepeater.inUart2.sendIndex          = 0;

	
}

s8 repeater_init(void)
{
	
	repeater_stat_init();
	
#if 1	
	DEBUG("out:%bd in1:%bd in2:%bd\r\n",uartRepeater.outUart.conUart,\
		uartRepeater.inUart1.baud.uartPort,uartRepeater.inUart2.baud.uartPort);
#endif

	//outUart init
	uartRepeater.outUart.uart.baud.baud     = BAUD115200;
	uartRepeater.outUart.uart.send_fun      = uart4_send;
#if 0	
	uartRepeater.outUart.presTimer          = creat_timer(500, 0, outuart_response_timer_out_callback , &uartRepeater);
	if(NULL == uartRepeater.outUart.presTimer)
	{
		return -1;
	}
	uartRepeater.outUart.uart.pTimer        = creat_timer(10, 0, outUart_timer_callback , &uartRepeater);
	if(NULL == uartRepeater.outUart.uart.pTimer)
	{
		return -1;
	}
#endif

	//inUart1 init
	uartRepeater.inUart1.baud.baud          = BAUD115200;
	uartRepeater.inUart1.send_fun           = uart2_send;
#if 0	
	uartRepeater.inUart1.pTimer             = creat_timer(10, 0, inUart_timer_callback , &uartRepeater.inUart1);
	if(NULL == uartRepeater.inUart1.pTimer)
	{
		return -1;
	}
#endif
	//inUart2 init
	uartRepeater.inUart2.baud.baud          = BAUD115200;
	uartRepeater.inUart2.send_fun           = uart3_send;
#if 0	
	uartRepeater.inUart2.pTimer             = creat_timer(10, 0, inUart_timer_callback , &uartRepeater.inUart2);
	if(NULL == uartRepeater.inUart2.pTimer)
	{
		return -1;
	}
#endif	
	return 0;
}



//08 01 00 00 00 20 3D 4B
//08 01 04 00 00 00 00 62 D1 


//08 03 00 00 00 20 44 8B
//08 83 03 D1 33


//01 03 00 00 00 01 84 0A


//01 01 00 00 00 20 3D D2

