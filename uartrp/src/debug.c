#include "STC15W4K.h"      // 包含 "STC15W4K.H"寄存器定义头文件
#include "uart_repeater.h" 
#include "uart1.h"


extern void uart1_send_byte(u8 byte);

void SendString(u8 *s)
{
    while (*s)                  //¼ì²â×Ö·û´®½áÊø±êÖ¾
    {
        uart1_send_byte(*s++);         //·¢ËÍµ±Ç°×Ö·û
    }
}

