#include "STC15W4K.h"      // 包含 "STC15W4K.H"寄存器定义头文件
#include "config.h"
#include "uart_repeater.h" 
#include "eeprom.h"
#include "intrins.h"


#if 0
#define IAP_CMD_MASK  0x03

#define READ_CMD()           IAP_CMD = 0x01
#define PROGRAM_CMD()        IAP_CMD = 0x02
#define ERASE_CMD()          IAP_CMD = 0x03
#define ENABLE_EEPROM_CTR()  IAP_CONTR |= 0x80
#define DISABLE_EEPROM_CTR() IAP_CONTR &= ~0x80

#define PARAM_ADDR_H         0x00
#define PARAM_ADDR_L         0x00



#define TRIG_EEPROM()    {ENABLE_EEPROM_CTR(); IAP_TRIG = 0x5A;IAP_TRIG = 0xA5;}


u8 eeprom_read(UART_BAUD *pbaud)
{
	READ_CMD();
	TRIG_EEPROM();

	return 0;
}
u8 eeprom_write(UART_BAUD *pbaud)
{
	ERASE_CMD();
	TRIG_EEPROM();
	PROGRAM_CMD();
	TRIG_EEPROM();

	return 0;
}
#endif



//sfr IAP_CMD   = 0xC5;
#define		IAP_STANDBY()	IAP_CMD = 0		//ISP空闲命令
#define		IAP_READ()		IAP_CMD = 1		
#define		IAP_WRITE()		IAP_CMD = 2		
#define		IAP_ERASE()		IAP_CMD = 3		

//sfr IaP_TRIG  = 0xC6;
#define 	IAP_TRIG()	IAP_TRIG = 0x5A,IAP_TRIG = 0xA5		//触发对eeprom操作命令

//							  7    6    5      4    3    2    1     0    Reset Value
//sfr IAP_CONTR = 0xC7;		IAPEN SWBS SWRST CFAIL  -   WT2  WT1   WT0   0000,x000	//IAP Control Register
#define IAP_EN			(1<<7)
#define IAP_SWBS		(1<<6)
#define IAP_SWRST		(1<<5)
#define IAP_CMD_FAIL	(1<<4)
#define IAP_WAIT_1MHZ	7
#define IAP_WAIT_2MHZ	6
#define IAP_WAIT_3MHZ	5
#define IAP_WAIT_6MHZ	4
#define IAP_WAIT_12MHZ	3
#define IAP_WAIT_20MHZ	2
#define IAP_WAIT_24MHZ	1
#define IAP_WAIT_30MHZ	0

#if (MAIN_Fosc >= 24000000L)
	#define		IAP_WAIT_FREQUENCY	IAP_WAIT_30MHZ
#elif (MAIN_Fosc >= 20000000L)
	#define		IAP_WAIT_FREQUENCY	IAP_WAIT_24MHZ
#elif (MAIN_Fosc >= 12000000L)
	#define		IAP_WAIT_FREQUENCY	IAP_WAIT_20MHZ
#elif (MAIN_Fosc >= 6000000L)
	#define		IAP_WAIT_FREQUENCY	IAP_WAIT_12MHZ
#elif (MAIN_Fosc >= 3000000L)
	#define		IAP_WAIT_FREQUENCY	IAP_WAIT_6MHZ
#elif (MAIN_Fosc >= 2000000L)
	#define		IAP_WAIT_FREQUENCY	IAP_WAIT_3MHZ
#elif (MAIN_Fosc >= 1000000L)
	#define		IAP_WAIT_FREQUENCY	IAP_WAIT_2MHZ
#else
	#define		ISP_WAIT_FREQUENCY	IAP_WAIT_1MHZ
#endif


#define		IAP_ENABLE()	IAP_CONTR = (IAP_EN + IAP_WAIT_FREQUENCY)
#define		IAP_DISABLE()	IAP_CONTR = 0; IAP_CMD = 0; IAP_TRIG = 0; IAP_ADDRH = 0xff; IAP_ADDRL = 0xff




//========================================================================
// 函数: void DisableEEPROM(void)
// 描述: 禁止EEPROM.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2014-6-30
//========================================================================
void DisableEEPROM(void)		//禁止访问EEPROM
{
	IAP_CONTR = 0;				//禁止ISP/IAP操作
	IAP_CMD   = 0;				//去除ISP/IAP命令
	IAP_TRIG  = 0;				//防止ISP/IAP命令误触发
	IAP_ADDRH = 0xff;			//指向非EEPROM区，防止误操作
	IAP_ADDRL = 0xff;			//指向非EEPROM区，防止误操作
}

//========================================================================
// 函数: void	EEPROM_Trig(void)
// 描述: 触发EEPROM操作.
// 参数: none.
// 返回: none.
// 版本: V1.0, 2014-6-30
//========================================================================
void	EEPROM_Trig(void)
{
	F0 = EA;	//保存全局中断
	EA = 0;		//禁止中断, 避免触发命令无效
	IAP_TRIG();							//先送5AH，再送A5H到ISP/IAP触发寄存器，每次都需要如此
										//送完A5H后，ISP/IAP命令立即被触发启动
										//CPU等待IAP完成后，才会继续执行程序。
	_nop_();
	_nop_();
	EA = F0;	//恢复全局中断
}


//========================================================================
// 函数: void	eeprom_erase_sector(u16 EE_address)
// 描述: 擦除一个扇区.
// 参数: EE_address:  要擦除的EEPROM的扇区中的一个字节地址.
// 返回: none.
// 版本: V1.0, 2014-6-30
//========================================================================
void	eeprom_erase_sector(u16 EE_address)
{
	IAP_ENABLE();						//设置等待时间，允许ISP/IAP操作，送一次就够
	IAP_ERASE();						//宏调用, 送扇区擦除命令，命令不需改变时，不需重新送命令
										//只有扇区擦除，没有字节擦除，512字节/扇区。
										//扇区中任意一个字节地址都是扇区地址。
	IAP_ADDRH = EE_address / 256;		//送扇区地址高字节（地址需要改变时才需重新送地址）
	IAP_ADDRL = EE_address % 256;		//送扇区地址低字节
	EEPROM_Trig();						//触发EEPROM操作
	DisableEEPROM();					//禁止EEPROM操作
}

//========================================================================
// 函数: void eeprom_read(u16 EE_address,u8 *DataAddress,u8 lenth)
// 描述: 读N个字节函数.
// 参数: EE_address:  要读出的EEPROM的首地址.
//       DataAddress: 要读出数据的指针.
//       length:      要读出的长度
// 返回: 0: 写入正确.  1: 写入长度为0错误.  2: 写入数据错误.
// 版本: V1.0, 2014-6-30
//========================================================================
void eeprom_read(u16 EE_address,u8 *DataAddress,u8 length)
{
	IAP_ENABLE();							//设置等待时间，允许ISP/IAP操作，送一次就够
	IAP_READ();								//送字节读命令，命令不需改变时，不需重新送命令
	do
	{
		IAP_ADDRH = EE_address / 256;		//送地址高字节（地址需要改变时才需重新送地址）
		IAP_ADDRL = EE_address % 256;		//送地址低字节
		EEPROM_Trig();						//触发EEPROM操作
		*DataAddress = IAP_DATA;			//读出的数据送往
		EE_address++;
		DataAddress++;
	}while(--length);

	DisableEEPROM();
}


//========================================================================
// 函数: u8 eeprom_write(u16 EE_address,u8 *DataAddress,u8 length)
// 描述: 写N个字节函数.
// 参数: EE_address:  要写入的EEPROM的首地址.
//       DataAddress: 要写入数据的指针.
//       length:      要写入的长度
// 返回: 0: 写入正确.  1: 写入长度为0错误.  2: 写入数据错误.
// 版本: V1.0, 2014-6-30
//========================================================================
u8 	eeprom_write(u16 EE_address,u8 *DataAddress,u8 length)
{
	u8	i;
	u16	j;
	u8	*p;
	
	if(length == 0)	return 1;	//长度为0错误

	IAP_ENABLE();						//设置等待时间，允许ISP/IAP操作，送一次就够
	i = length;
	j = EE_address;
	p = DataAddress;
	IAP_WRITE();							//宏调用, 送字节写命令
	do
	{
		IAP_ADDRH = EE_address / 256;		//送地址高字节（地址需要改变时才需重新送地址）
		IAP_ADDRL = EE_address % 256;		//送地址低字节
		IAP_DATA  = *DataAddress;			//送数据到ISP_DATA，只有数据改变时才需重新送
		EEPROM_Trig();						//触发EEPROM操作
		EE_address++;						//下一个地址
		DataAddress++;						//下一个数据
	}while(--length);						//直到结束

	EE_address = j;
	length = i;
	DataAddress = p;
	i = 0;
	IAP_READ();								//读N个字节并比较
	do
	{
		IAP_ADDRH = EE_address / 256;		//送地址高字节（地址需要改变时才需重新送地址）
		IAP_ADDRL = EE_address % 256;		//送地址低字节
		EEPROM_Trig();						//触发EEPROM操作
		if(*DataAddress != IAP_DATA)		//读出的数据与源数据比较
		{
			i = 2;
			break;
		}
		EE_address++;
		DataAddress++;
	}while(--length);

	DisableEEPROM();
	return i;
}




u8 save_buad(UART_BAUD *pbaud)
{
	u8 res = 2;
	
	if(pbaud)
	{
		eeprom_erase_sector(PARAM_SAVE_ADDR);
		res = eeprom_write(PARAM_SAVE_ADDR,(u8 *)pbaud,sizeof(UART_BAUD));
	}
	return res;
}



u8 read_buad(UART_BAUD *pbaud)
{
	u8 res = 0;
	
	if(pbaud)
	{
		eeprom_read(PARAM_SAVE_ADDR,(u8 *)pbaud,sizeof(UART_BAUD));
	}
	return res;
}




