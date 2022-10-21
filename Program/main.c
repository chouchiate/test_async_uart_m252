/**
 * @file main.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "includes.h"

void DescriptionMessage(void)
{
    printf("+----------------------------------------------------------------------+\n");
    printf("|  This sample code performs how to receive unknow data length package.|\n");
    printf("|                                                                      |\n");
    printf("|   (1) Please send data to UART0 Rx(PB.12)                            |\n");
    printf("|   (2) UART will receive data by UART Rx RDA and RXTO interrupt.      |\n");
    printf("|   (3) User can modify the Rx Timeout counter RX_TIMEOUT_CNT for      |\n");
    printf("|       diffirent timeout period.                                      |\n");
    printf("|                                                                      |\n");
    printf("|   Description for RX_TIMEOUT_CNT :                                   |\n");
    printf("|   -UART data = 8 bits                                                |\n");
    printf("|   -UART Parity = None                                                |\n");
    printf("|   -RX_TIMEOUT_CNT = 60                                               |\n");
    printf("|     If there is no data comes in 60 baudrate clock,                  |\n");
    printf("|     the UART Rx timeout interrupt flag will be set to 1.             |\n");
    printf("|     RX_TIMEOUT_CNT = 60 = 6 * ( 1 start + 8 data bits + 1 stop bit ) |\n");
    printf("|                         = 6 bytes data transmittion times            |\n");
    printf("+----------------------------------------------------------------------+\n\n");

    UART_WAIT_TX_EMPTY(UART0);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
    uint32_t u32ResetBufferIndex;
    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    /* Init UART */
    UART0_Init();

    /* Print description message */
    DescriptionMessage();

    while (1)
    {
        /* Wait to receive UART data */
        while (UART_RX_IDLE(UART0));

        /* Start to received UART data */
        g_bUART_RX_Received_Data_State = eUART_RX_Received_Data_NOT_Finish;

        /* Wait for receiving UART message finished */
        while (g_bUART_RX_Received_Data_State != eUART_RX_Received_Data_Finish);

        printf("\nUART0 Rx Received Data : %s\n", g_au8UART_RX_Buffer);
        printf("UART0 Rx RDA (Fifofull) interrupt times : %d\n", g_u8UART_RDA_Trigger_Cnt);
        printf("UART0 Rx RXTO (Timeout) interrupt times : %d\n", g_u8UART_RXTO_Trigger_Cnt);

        /* Reset SRAM Buffer */
        u32ResetBufferIndex = 0;

        while (g_au8UART_RX_Buffer[u32ResetBufferIndex] != '\0')
            g_au8UART_RX_Buffer[u32ResetBufferIndex++] = '\0';

        /* Reset UART interrupt parameter */
        UART_EnableInt(UART0, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk);
        g_u8UART_RDA_Trigger_Cnt = 0; // UART RDA interrupt times
        g_u8UART_RXTO_Trigger_Cnt = 0; // UART RXTO interrupt times
    }
}
