/* DC motor control based on STM32F103
 *
 * R.Grbic, 2020.
 *
 */


/* Includes */
#include <stddef.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "uart.h"
#include "stm32f103_misc.h"
#include "stm32f10x_util.h"
#include "stm32f10x_ina219.h"

#include<stdio.h>
#include<stdlib.h>

/* global variables */
volatile char receivedChar;
volatile uint16_t currentPWM;
extern const uint16_t PWM_period;
volatile int noMs = 0;
float voltage, current;
char *buff;
volatile int flagMeasure = 0;

volatile uint16_t pulse_ticks = 0;
volatile unsigned long start_time = 0;
volatile unsigned long end_time = 0;

// moje varijable

float motor_speed = 0;

float num_of_encoder_ticks = 0;
float prbs_width_in_ticks = 35;
float prbs_tick_counter = 0;
int myArray[3] = { 9800, 6950, 13200 };

/* UART receive interrupt handler */
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE))
	{
		//echo character
		receivedChar = USART_GetChar();
		if(receivedChar == 'u')
		{
			currentPWM = Get_PWM();
			currentPWM += (uint16_t)500;
			if(currentPWM < PWM_period)
				Set_PWM(currentPWM);
		}
		else if(receivedChar == 'd')
		{

			currentPWM = Get_PWM();
			if(currentPWM > 7000u)
			{
				currentPWM -= (uint16_t)500;
				Set_PWM(currentPWM);
			}
		}

		USART_PutChar(receivedChar);
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}


/* TIM2 input capture interrupt */
/* Okida svaki put kada dođe do rising-edgea */
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_CC4))
	{
		end_time = TIM2->CCR4;
		pulse_ticks = end_time - start_time;
        start_time = end_time;
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC4);

		//za brzinu vrtnje
		num_of_encoder_ticks++;
	}

}


/* TIMER4 every 0.1 second interrupt --> sending data to PC */
/* */
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update))
	{
		USART_PutChar('p');
		USART_PutChar('m');


		prbs_tick_counter++;

		//salje signal UARTom na SerialPort
		USART_SendUInt_32(Get_PWM());
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

		//Brzina vrtnje
		motor_speed = (num_of_encoder_ticks / 0.01 ) * 60 / 41;
		num_of_encoder_ticks = 0;
		USART_SendUInt_32(motor_speed);

		//prbs_width_in_ticks
		//prbs_counter
		/*
		prbs_tick_counter++;
		if (prbs_tick_counter == prbs_width_in_ticks)
		{
			int randomIndex = rand() % 3;
			//uint16_t randomValue =;
			Set_PWM((uint16_t)myArray[randomIndex]);
			Delay_ms(50);
			prbs_tick_counter = 0;
		}
		*/
		//int randomIndex = rand() % 3;
		//Set_PWM((uint16_t)myArray[randomIndex]);

		//Delay_ms(3000);



				/*
				srand ( time(NULL) );
				int myArray[3] = { 9800, 6950, 13200 };
				int randomIndex = rand() % 3;
				int randomValue = myArray[randomIndex];
				printf("Reference: %d\n", randomValue);
				return 0;
				 */

	}
}


/* systick timer for periodic tasks */
void SysTick_Handler(void)
{
	noMs++;
}


/* main program - init peripherals and loop */
int main(void)
{
	NVIC_SetPriorityGrouping(0u);
	Systick_init();
	Output_setup();
	USART1_PC_Init();
	Timer_setup();
	Button_init();

	while (1)
	{

		if (prbs_tick_counter == prbs_width_in_ticks)
				{
					int randomIndex = rand() % 3;
					//uint16_t randomValue =;
					Set_PWM((uint16_t)myArray[randomIndex]);
					Delay_ms(50);
					prbs_tick_counter = 0;
				}
		if ()
		// read push button - stop motor
		if(!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14))
		{
			Set_PWM(0u);
			Delay_ms(50);
		}
		// read push button - turn on motor
		if(!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15))
		{
			Set_PWM(13200u);
			Delay_ms(50);
		}

	}
}

