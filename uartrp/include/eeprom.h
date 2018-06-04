#ifndef __EEPROM_H_
#define __EEPROM_H_


#include "STC15W4K.h"      // 包含 "STC15W4K.H"寄存器定义头文件
#include	"config.h"



extern void eeprom_erase_sector(u16 EE_address);
extern void eeprom_read(u16 EE_address,u8 *DataAddress,u8 length);
extern u8 	eeprom_write(u16 EE_address,u8 *DataAddress,u8 length);

extern u8  save_buad(UART_BAUD *pbaud);
extern u8  read_buad(UART_BAUD *pbaud);


#endif

