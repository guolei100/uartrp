#include "uart_repeater.h" 
#include "uart1.h"
#include "uart2.h"
#include "uart3.h"
#include "uart4.h"
#include "debug.h"
#include <string.h>
#include "delay.h"
#include "eeprom.h"


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


void prouart_isr(u8 recByte, UART_WAY *pProUart)
{
	MUTEX_LOCK();
	
	
	if(pProUart->revIndex >= sizeof(pProUart->revBuf) )
	{
		pProUart->revIndex = 0;
	}
	pProUart->revBuf[pProUart->revIndex] = recByte;
	pProUart->revIndex++;
	pProUart->revLen++;
	pProUart->recStat = FRAME_RECEIVING;
	
	MUTEX_UNLOCK();
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


void repeater_transmiting(void)
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




u8 strncasecmp(const char *s1, const char *s2, size_t n)  
{  
    u8 c1 = 0, c2 = 0; 


	
    while(n--)  
    {  
        c1 = *s1++;  
        c2 = *s2++;
        if(!c1 || !c2) break;  
        if(c1>='A'&&c1<='Z') c1 += 'a' - 'A';  
        if(c2>='A'&&c2<='Z') c2 += 'a' - 'A';  
        if(c1!=c2) break;  
    } 
	
    return c1-c2;  
}  


u8 isspace(u8 c)
{
	if(c =='\t'|| c =='\n' ||  c =='\r'|| c ==' ')
		return 1;
	else
		return 0;
}
u8 isdigit(u8 c)
{
    return (c >= '0' && c <= '9');
}


s32  atol(  
        const char *nptr  
        )  
{  
        int c;              /* current char */  
        s32 total;         /* current total */  
        int sign;           /* if '-', then negative, otherwise positive */  
  
        /* skip whitespace */  
        while ( isspace((int)(unsigned char)*nptr) )  
            ++nptr;  
  
        c = (int)(unsigned char)*nptr++;  
        sign = c;           /* save sign indication */  
        if (c == '-' || c == '+')  
            c = (int)(unsigned char)*nptr++;    /* skip sign */  
  
        total = 0;  
  
        while (isdigit(c)) {  
            total = 10 * total + (c - '0');     /* accumulate digit */  
            c = (int)(unsigned char)*nptr++;    /* get next char */  
        }  
  
        if (sign == '-')  
            return -total;  
        else  
            return total;   /* return result, negated if necessary */  
}  


u8 get_wifi_answer(void)
{
	if(FRAME_RECEIVED == uartRepeater.inUart1.recStat)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


u8 wait_wifi_answer(void)
{
	u8 cnt = 200;
	u8 res = 0;
	
	while(get_wifi_answer())
	{
		if(0 == cnt--)
		{
			cnt = 0xff;
			break;
		}
		delay_ms(2);
	}
	if(0xff != cnt)
	{
		res = 1;
	}

	return res;
}

void clear_wifi_uart(void)
{
	memset(uartRepeater.inUart1.revBuf, 0, sizeof(uartRepeater.inUart1.revBuf));
	uartRepeater.inUart1.revIndex = 0;
	uartRepeater.inUart1.revLen = 0;
	uartRepeater.inUart1.recStat = NO_RECEIVE;

	uartRepeater.inUart1.sendIndex = 0;
	uartRepeater.inUart1.sendLen = 0;
	uartRepeater.inUart1.sendStat = NO_SEND;
}

void clear_pro_uart(void)
{
	memset(uartRepeater.proUart.revBuf, 0, sizeof(uartRepeater.proUart.revBuf));
	uartRepeater.proUart.revIndex = 0;
	uartRepeater.proUart.revLen = 0;
	uartRepeater.proUart.recStat = NO_RECEIVE;

	uartRepeater.proUart.sendIndex = 0;
	uartRepeater.proUart.sendLen = 0;
	uartRepeater.proUart.sendStat = NO_SEND;
}


u8 send_cmd_wifi(u8 *pSndDat, u32 len)
{
	u16 cnt = 0;
	u8 errCode = 0;

	DEBUG("send cmd to wifi\r\n");
	if(uartRepeater.outUart.conUart == uartRepeater.inUart1.baud.uartPort)
	{
		uartRepeater.outUart.conUart = NONE_UART;
	}
	
	clear_wifi_uart();
	uartRepeater.wifiCommStep = 1;
	if(uartRepeater.inUart1.send_fun)
		uartRepeater.inUart1.send_fun("+++",3);//进入命令模式

	//wifto into cmd mode
	errCode = 1;
	if(wait_wifi_answer())
	{
		if('a'== uartRepeater.inUart1.revBuf[0])
		{
			uartRepeater.wifiCommStep++;
		}
	}
	
	
	clear_wifi_uart();
	if(2 == uartRepeater.wifiCommStep)
	{
		if(uartRepeater.inUart1.send_fun)
			 uartRepeater.inUart1.send_fun("a",1);
		if(wait_wifi_answer())
		{
			if( ('+' == uartRepeater.inUart1.revBuf[0]) && \
				('o' == uartRepeater.inUart1.revBuf[1]) && \
				('k' == uartRepeater.inUart1.revBuf[2]) )
			{//wifi into cmd mode
				uartRepeater.wifiCommStep++;
				errCode = 0;
			}
			
		}
	}

	if(0==errCode)
		errCode = 2;
	//send cmd to wifi
	clear_wifi_uart();
	if(3 == uartRepeater.wifiCommStep)
	{
		if(uartRepeater.inUart1.send_fun)
			uartRepeater.inUart1.send_fun(pSndDat,len);

		delay_ms(200);
		delay_ms(200);
		delay_ms(200);
	
		if(wait_wifi_answer())
		{
			if(uartRepeater.proUart.send_fun)
				uartRepeater.proUart.send_fun(uartRepeater.inUart1.revBuf,uartRepeater.inUart1.revLen);
			uartRepeater.wifiCommStep++;
			errCode = 0;
		}
	}

	
	if(0==errCode)
		errCode = 3;

	clear_wifi_uart();
	//wifi exit from cmd mode
	if(4 == uartRepeater.wifiCommStep)
	{
		if(uartRepeater.inUart1.send_fun)
			uartRepeater.inUart1.send_fun("AT+ENTM\r",8);//退出命令模式

		delay_ms(100);
		//delay_ms(200);
		if(wait_wifi_answer())
		{
			if( strstr(uartRepeater.inUart1.revBuf,"+ok") )
			{//wifi into cmd mode
				uartRepeater.wifiCommStep++;
				errCode = 0;
			}
			else
			{
				DEBUG("answer AT+ENTM cmd: ");
				uart1_send(uartRepeater.inUart1.revBuf,uartRepeater.inUart1.revLen);
			}
			
		}
		
	}
	switch(errCode)
	{
		case 1:
			if(uartRepeater.proUart.send_fun)
				uartRepeater.proUart.send_fun("wifi module into cmd mode fail",uartRepeater.inUart1.revLen);
			break;
		case 2:
			if(uartRepeater.proUart.send_fun)
				uartRepeater.proUart.send_fun("wifi module refusing to carry out orders",uartRepeater.inUart1.revLen);
			break;
		case 3:
			if(uartRepeater.proUart.send_fun)
				uartRepeater.proUart.send_fun("wifi module exit from cmd mode fail",uartRepeater.inUart1.revLen);
			break;
		default:
			DEBUG("send cmd to successfully\r\n");
			break;
	}
	clear_wifi_uart();
	clear_pro_uart();
	uartRepeater.mode = NORMAL_COM;
	uartRepeater.wifiCommStep = 0;

	return errCode;
	
}

u32 calc_sum(UART_BAUD *pbaud);
void print_buad(UART_BAUD *pbaud);


void explain_pro(void)
{
	u8 i = 0;
	u8 *p;
	u8 *pOddEven;
	
	if(FRAME_RECEIVED == uartRepeater.proUart.recStat)
	{
		uartRepeater.proUart.recStat = NO_RECEIVE;
		
		//DEBUG("rev data %hd:",uartRepeater.proUart.revLen);
		//uart1_send(uartRepeater.proUart.revBuf,uartRepeater.proUart.revLen);
		//DEBUG(" \r\n");
		if(8==uartRepeater.proUart.revLen)
		{
			//DEBUG("get uart baud\r\n");
			if(0 == strncasecmp("AT+UART",uartRepeater.proUart.revBuf,7))
			{//查询波特率(本地的而非wifi的)
				uartRepeater.mode = SET_UP;
				send_cmd_wifi("AT+UART\r",8);
			}
		}
		//DEBUG("wait =\r\n");
		if(0 == strncasecmp("AT+Uart=",uartRepeater.proUart.revBuf,8))
		{
			//DEBUG("set uart baud\r\n");
			i=0;
			p = uartRepeater.proUart.revBuf;
			while(1)
			{
				pOddEven = p;
				p = strchr(p,',');
				if(p)
				{
					p++;
				}
				else
				{
					break;
				}
				i++;
			}
			
			if(3 ==i)
			{
				uartRepeater.proUart.baud.baud = atol(&uartRepeater.proUart.revBuf[8]);
				if(uartRepeater.proUart.baud.baud)
				{
					uartRepeater.proUart.baud.dataLen = 8;
					uartRepeater.proUart.baud.stopBits = 1;
					//DEBUG("baud= %ld\r\n",uartRepeater.proUart.baud.baud);
					if(0 == strncasecmp("NONE",pOddEven,4))
					{
						uartRepeater.proUart.baud.oddEven = NONE_ODD_EVEN;
						uartRepeater.mode = SET_UP;
					}
					else if(0 == strncasecmp("EVEN",pOddEven,4))
					{
						uartRepeater.proUart.baud.oddEven = EVEN;
						uartRepeater.mode = SET_UP;
					}
					else if(0 == strncasecmp("ODD",pOddEven,3))
					{
						uartRepeater.proUart.baud.oddEven = ODD;
						uartRepeater.mode = SET_UP;
					}
					else if(0 == strncasecmp("MARK",pOddEven,4))
					{
						uartRepeater.proUart.baud.oddEven = MARK;
						uartRepeater.mode = SET_UP;
					}
					else if(0 == strncasecmp("SPACE",pOddEven,5))
					{
						uartRepeater.proUart.baud.oddEven = SPACE;
						uartRepeater.mode = SET_UP;
					}
					else
					{
						DEBUG("ODD EVEN ERR\r\n");
					}
					if(SET_UP == uartRepeater.mode)
					{
						if(0 == send_cmd_wifi(uartRepeater.proUart.revBuf,uartRepeater.proUart.revLen) )
						{
							uartRepeater.proUart.baud.saveMark = SAVE_MARK;
							uartRepeater.proUart.baud.sum = calc_sum(&uartRepeater.proUart.baud);
							
							if(save_buad(&uartRepeater.proUart.baud) )
							{
								uart1_send("save buad fail\r\n",16);
							}
							else
							{	
								uart1_send("save buad successfully\r\n",24);
							}
						}
					}
					
				}
				
			}
		}
		
	}
}

void repeater_running(void)
{
	if(NORMAL_COM == uartRepeater.mode)
	{
		repeater_transmiting();
	}
	explain_pro();
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

	uartRepeater.proUart.sendStat           = NO_SEND;
	uartRepeater.proUart.recStat            = NO_RECEIVE;
	uartRepeater.proUart.baud.uartPort      = UART1_PORT;
	uartRepeater.proUart.revLen             = 0;
	uartRepeater.proUart.sendLen            = 0;
	uartRepeater.proUart.revIndex           = 0;
	uartRepeater.proUart.sendIndex          = 0;

	
}



u32 calc_sum(UART_BAUD *pbaud)
{
	u32 sum = 0;
	
	if(pbaud)
	{
		sum += pbaud->uartPort;
		sum += pbaud->baud;
		sum += pbaud->dataLen;
		sum += pbaud->oddEven;
		sum += pbaud->stopBits;
	}
	return sum;
}
void buad_cpy(UART_BAUD *pBaudSrc,UART_BAUD *pBaudTo)
{
	if(pBaudSrc && pBaudTo)
	{
		pBaudTo->baud     = pBaudSrc->baud;
		pBaudTo->dataLen  = pBaudSrc->dataLen;
		pBaudTo->oddEven  = pBaudSrc->oddEven;
		pBaudTo->stopBits = pBaudSrc->stopBits;

		pBaudTo->saveMark = pBaudSrc->saveMark;
		pBaudTo->sum      = pBaudSrc->sum;
	}
}


void uart_baud_defult(void)
{
	uartRepeater.proUart.baud.baud     = 9600;
	uartRepeater.proUart.baud.dataLen  = 8;
	uartRepeater.proUart.baud.oddEven  = ODD;
	uartRepeater.proUart.baud.stopBits = 1;
	uartRepeater.proUart.baud.saveMark = SAVE_MARK;
	uartRepeater.proUart.baud.sum      = calc_sum(&uartRepeater.proUart.baud);
	buad_cpy(&uartRepeater.proUart.baud,     &uartRepeater.inUart1.baud);
	buad_cpy(&uartRepeater.proUart.baud,     &uartRepeater.inUart2.baud);
	buad_cpy(&uartRepeater.proUart.baud,     &uartRepeater.outUart.uart.baud);
}

void uart_init(void)
{
	u32 sum;
	u8 err = 0;
	
	if(SAVE_MARK == uartRepeater.proUart.baud.saveMark)
	{
		sum = calc_sum(&uartRepeater.proUart.baud);
		if(uartRepeater.proUart.baud.sum == sum)
		{
			buad_cpy(&uartRepeater.proUart.baud,   &uartRepeater.inUart1.baud);
			buad_cpy(&uartRepeater.proUart.baud,   &uartRepeater.inUart2.baud);
			buad_cpy(&uartRepeater.proUart.baud,   &uartRepeater.outUart.uart.baud);
			
		}
		else
		{
			err = 1;
			uart_baud_defult();
			
		}
	}
	else
	{
		err = 2;
		uart_baud_defult();
	}
	uart2_init(uartRepeater.proUart.baud.baud,uartRepeater.proUart.baud.oddEven);
	uart1_init(uartRepeater.proUart.baud.oddEven);
	uart4_init(uartRepeater.proUart.baud.oddEven);
	uart3_init(uartRepeater.proUart.baud.oddEven);
	switch(err)
	{
		case 0:
			DEBUG("uart baud read from eeprom\r\n");
			break;
		case 1:
			DEBUG("uart baud sum is err\r\n");
			break;
		case 2:
			DEBUG("uart baud is default\r\n");
			break;
	}
}




void print_buad(UART_BAUD *pbaud)
{
	if(pbaud)
	{
		DEBUG("baud:%ld,",pbaud->baud);
		DEBUG("%bd,",pbaud->dataLen);
		DEBUG("%bd,",pbaud->stopBits);
		if(NONE_ODD_EVEN == pbaud->oddEven)
		{
			DEBUG("NONE");
		}
		else if(ODD == pbaud->oddEven)
		{
			DEBUG("ODD");
		}
		else if(EVEN == pbaud->oddEven)
		{
			DEBUG("EVEN");
		}
		else if(MARK == pbaud->oddEven)
		{
			DEBUG("MARK");
		}
		else if(SPACE == pbaud->oddEven)
		{
			DEBUG("SPACE");
		}
		else
		{
			DEBUG("err");
		}
		DEBUG("\r\n");
	}
}
s8 repeater_init(void)
{
	read_buad(&uartRepeater.proUart.baud);
	uart_init();
	repeater_stat_init();

	print_buad(&uartRepeater.outUart.uart.baud);
	print_buad(&uartRepeater.inUart1.baud);
	print_buad(&uartRepeater.inUart2.baud);
	print_buad(&uartRepeater.proUart.baud);
	
	uartRepeater.mode = NORMAL_COM;
	uartRepeater.wifiCommStep = 0;
	
#if 1	
	DEBUG("out:%bd in1:%bd in2:%bd\r\n",uartRepeater.outUart.conUart,\
		uartRepeater.inUart1.baud.uartPort,uartRepeater.inUart2.baud.uartPort);
#endif

	//outUart init

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
	uartRepeater.inUart1.send_fun           = uart2_send;
#if 0	
	uartRepeater.inUart1.pTimer             = creat_timer(10, 0, inUart_timer_callback , &uartRepeater.inUart1);
	if(NULL == uartRepeater.inUart1.pTimer)
	{
		return -1;
	}
#endif
	//inUart2 init
	uartRepeater.inUart2.send_fun           = uart3_send;
#if 0	
	uartRepeater.inUart2.pTimer             = creat_timer(10, 0, inUart_timer_callback , &uartRepeater.inUart2);
	if(NULL == uartRepeater.inUart2.pTimer)
	{
		return -1;
	}
#endif	

	uartRepeater.proUart.send_fun           = uart1_send;
	return 0;
}



//08 01 00 00 00 20 3D 4B
//08 01 04 00 00 00 00 62 D1 


//08 03 00 00 00 20 44 8B
//08 83 03 D1 33


//01 03 00 00 00 01 84 0A


//01 01 00 00 00 20 3D D2

