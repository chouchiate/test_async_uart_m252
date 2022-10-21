#ifndef __SYS_UART0_H__
#define __SYS_UART0_H__

#include <machine/_default_types.h>

#define RX_BUFFER_SIZE 128
#define RX_TIMEOUT_CNT 60 //40~255
#define FIFO_THRESHOLD 4

extern volatile uint8_t g_au8UART_RX_Buffer[RX_BUFFER_SIZE]; // UART Rx received data Buffer (RAM)
extern volatile uint8_t g_bUART_RX_Received_Data_State;
extern volatile uint8_t g_u8UART_RDA_Trigger_Cnt; // UART RDA interrupt trigger times counter
extern volatile uint8_t g_u8UART_RXTO_Trigger_Cnt; // UART RXTO interrupt trigger times counter


void UART0_Init(void);

#endif /* __SYS_UART0_H__ */