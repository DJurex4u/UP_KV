/*
 * uart.c
 *
 *  Created on: Jun 1, 2018
 *      Author: rgrbic
 */

#include "uart.h"


void USART1_PC_Init(void)
{
    /* USART configuration structure for USART1 */
    USART_InitTypeDef USART1_InitStruct;
    /* Bit configuration structure for GPIOA PIN9 and PIN10 */
    GPIO_InitTypeDef GPIO_USART1_InitStruct;

    /* Enable clock for USART1, AFIO and GPIOA */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO |
                           RCC_APB2Periph_GPIOA, ENABLE);

    /* GPIOA PIN9 alternative function Tx */
    GPIO_USART1_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_USART1_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_USART1_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_USART1_InitStruct);
    /* GPIOA PIN10 alternative function Rx */
    GPIO_USART1_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_USART1_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_USART1_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_USART1_InitStruct);

    /* Baud rate 9600, 8-bit data, One stop bit
     * No parity, Do both Rx and Tx, No HW flow control
     */
    USART1_InitStruct.USART_BaudRate = 115200;
    USART1_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART1_InitStruct.USART_StopBits = USART_StopBits_1;
    USART1_InitStruct.USART_Parity = USART_Parity_No ;
    USART1_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART1_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    /* Configure USART1 */
    USART_Init(USART1, &USART1_InitStruct);

    /* Enable USART1 */
    USART_Cmd(USART1, ENABLE);

    /* Enable RXNE interrupt */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    /* set priority */
    NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),3,0));

    /* Enable USART1 global interrupt */
    NVIC_EnableIRQ(USART1_IRQn);


}


void USART_PutChar(char c)
{
    // Wait until transmit data register is empty
    while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));
    // Send a char using USART1
    USART_SendData(USART1, c);
}


void USART_PutString(char *s)
{
    // Send a string
    while (*s)
    {
        USART_PutChar(*s++);
    }
}


uint16_t USART_GetChar(void)
{
    // Wait until data is received
    while (!USART_GetFlagStatus(USART1, USART_FLAG_RXNE));
    // Read received char
    return USART_ReceiveData(USART1);
}


void USART_SendUInt_16(uint16_t num)
{
	char c;

	c = num & 0xFF;
	USART_PutChar(c);

	c = (num >> 8) & 0xFF;
	USART_PutChar(c);
}

void USART_SendUInt_32(uint32_t num)
{
	char c;

	c = num & 0xFF;
	USART_PutChar(c);

	c = (num >> 8) & 0xFF;
	USART_PutChar(c);

	c = (num >> 16) & 0xFF;
	USART_PutChar(c);

	c = (num >> 24) & 0xFF;
	USART_PutChar(c);




}


//send signed integer value as text to UART
/*void UART_SendInt(int32_t num)
{
	char str[10]; // 10 chars max for INT32_MAX
	int i = 0;
	if (num < 0) {
		USART_PutChar('-');
		num *= -1;
	}
	do str[i++] = num % 10 + '0'; while ((num /= 10) > 0);
	while (i) USART_PutChar(str[--i]);
}*/



