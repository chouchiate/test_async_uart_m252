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

#include <stdio.h>
#include "NuMicro.h"

#define FIFO_THRESHOLD 4
#define RX_BUFFER_SIZE 128
#define RX_TIMEOUT_CNT 60 //40~255

enum
{
    eUART_RX_Received_Data_Finish = 0,
    eUART_RX_Received_Data_NOT_Finish
};

volatile uint8_t g_au8UART_RX_Buffer[RX_BUFFER_SIZE] = {0}; // UART Rx received data Buffer (RAM)
volatile uint8_t g_bUART_RX_Received_Data_State = eUART_RX_Received_Data_NOT_Finish;
volatile uint8_t g_u8UART_RDA_Trigger_Cnt = 0; // UART RDA interrupt trigger times counter
volatile uint8_t g_u8UART_RXTO_Trigger_Cnt = 0; // UART RXTO interrupt trigger times counter

void UART0_IRQHandler(void)
{
    uint8_t i;
    static uint16_t u16UART_RX_Buffer_Index = 0;

    if (UART_GET_INT_FLAG(UART0, UART_INTSTS_RDAINT_Msk))
    {
        /* UART receive data available flag */

        /* Record RDA interrupt trigger times */
        g_u8UART_RDA_Trigger_Cnt++;

        /* Move the data from Rx FIFO to sw buffer (RAM). */
        /* Every time leave 1 byte data in FIFO for Rx timeout */
        for (i = 0 ; i < (FIFO_THRESHOLD - 1) ; i++)
        {
            g_au8UART_RX_Buffer[u16UART_RX_Buffer_Index] = UART_READ(UART0);
            u16UART_RX_Buffer_Index ++;

            if (u16UART_RX_Buffer_Index >= RX_BUFFER_SIZE)
                u16UART_RX_Buffer_Index = 0;
        }
    }
    else if (UART_GET_INT_FLAG(UART0, UART_INTSTS_RXTOINT_Msk))
    {
        /* When Rx timeout flag is set to 1, it means there is no data needs to be transmitted. */

        /* Record Timeout times */
        g_u8UART_RXTO_Trigger_Cnt++;

        /* Move the last data from Rx FIFO to sw buffer. */
        while (UART_GET_RX_EMPTY(UART0) == 0)
        {
            g_au8UART_RX_Buffer[u16UART_RX_Buffer_Index] = UART_READ(UART0);
            u16UART_RX_Buffer_Index ++;
        }

        /* Clear UART RX parameter */

        UART_DISABLE_INT(UART0, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk);
        u16UART_RX_Buffer_Index = 0;
        g_bUART_RX_Received_Data_State = eUART_RX_Received_Data_Finish;
    }
}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable HIRC clock  */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Wait for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Enable LIRC clock */
    CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);

    /* Wait for LIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);

    /* Select HCLK clock source as HIRC and HCLK clock divider as 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Enable UART module clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Select UART module clock source as HIRC and UART module clock divider as 1 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Set PB multi-function pins for UART0 RXD=PB.12 and TXD=PB.13 */
    Uart0DefaultMPF();

    /* Lock protected registers */
    SYS_LockReg();
}

void UART0_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART0 */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);

    /* Enable UART RDA and RX timeout interrupt */
    UART_EnableInt(UART0, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk);
    NVIC_EnableIRQ(UART0_IRQn);

    /* Set RX Trigger Level as 4 bytes */
    UART0->FIFO = ((UART0->FIFO & (~UART_FIFO_RFITL_Msk)) | UART_FIFO_RFITL_4BYTES);

    /* Set Timeout time counter in 60 bit-time and enable time-out counter */
    UART_SetTimeoutCnt(UART0, RX_TIMEOUT_CNT);
}

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
