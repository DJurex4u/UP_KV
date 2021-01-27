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
float prbs_width_in_ticks = 30;
float prbs_tick_counter = 0;
int myArray[2] = {6500, 9500 };
int myArray2[4] = {30, 35, 40, 45};
int last = 0;


float w_zadano = 2500;
float w0 = 0;//3950;
float stari_pwm = 0;
float staro_reg_odstupanje = 0;

/* UART receive interrupt handler */
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE))
	{
		//echo character
		receivedChar = USART_GetChar();
		if(receivedChar == 'u')
		{
			w_zadano += 500;
		}
		else if(receivedChar == 'd')
		{
			w_zadano -= 500;
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

regulacija()
{
	double reg_odstupanje = w_zadano - (motor_speed + w0);
			double pwm_upravljacki = stari_pwm + 1.973 * reg_odstupanje - 1.48 * staro_reg_odstupanje;

			Set_PWM((uint16_t)pwm_upravljacki);

			stari_pwm = pwm_upravljacki;
			staro_reg_odstupanje = reg_odstupanje;
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
		USART_SendUInt_32(w_zadano);

		//Regulator
		regulacija();
	}

}

/* systick timer for periodic tasks */
void SysTick_Handler(void)
{
	noMs++;
}

	void prbs_function()
	{
		if (prbs_tick_counter == prbs_width_in_ticks)
			{
				if (last == 0)
					{
							last = 1;
							Set_PWM((uint16_t)myArray[1]);
							Delay_ms(50);
						}else if (last == 1)
						{
							last = 0;
							Set_PWM((uint16_t)myArray[0]);
							Delay_ms(50);
						}
						prbs_width_in_ticks = myArray2[rand() % 4];
						prbs_tick_counter = 0;
					}
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

		//prbs_function();


		// read push button - stop motor
		if(!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14))
		{
			Set_PWM(7000u);
			Delay_ms(50);
		}
		// read push button - turn on motor
		if(!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15))
		{
			Set_PWM(8000u);
			Delay_ms(50);
		}

	}
}

