/*
 * stm32f103_misc.c
 *
 *  Created on: Dec 12, 2020
 *      Author: rgrbic
 */

#include "stm32f103_misc.h"

const uint16_t PWM_period = 14400;
static uint32_t startTime = 0;
extern volatile int noMs;

void Systick_init(void)
{
	// interrupt every ms
	// RELOAD = (1 ms x 72 MHz) - 1 = 71999 ticks
	SysTick_Config(71999);
}

void Delay_ms(uint32_t delayMs)
{
	startTime = noMs;
	while( (noMs - startTime) < delayMs){}
}

void Output_setup(void)
{
	GPIO_InitTypeDef GPIO_InitStructure_LED;
	GPIO_InitTypeDef GPIO_InitStructure_L298N;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	//plava LED na PC13
	GPIO_InitStructure_LED.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure_LED.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure_LED.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure_LED);

	GPIO_SetBits(GPIOC,GPIO_Pin_13); //LED turn off


	//L298N IN1 IN2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure_L298N.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_10;
	GPIO_InitStructure_L298N.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure_L298N.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure_L298N);

	//motor forward
	GPIO_SetBits(GPIOB,GPIO_Pin_11);
	GPIO_ResetBits(GPIOB,GPIO_Pin_10);
}

void Timer_setup(void)
{
	 /*
	 * System clock frequency 72 MHz
	 * PLCK1 36 MHz
	 * PLCK2 72 MHz
	 * AHB prescaler 1
	 * APB1 prescaler /2
	 * APB1 36MHz
	 * APB2 72 MHz
	 * TIM2,3,4 additional prescaler x2 - input frequency 72 MHz
	 * TIM1 input frequency 72 MHz
	 *
	 * see RM0008 pp. 93/1132
	 */

	 /*
	 * UE [s] = ((PSC+1)*(TIM9_ARR+1))/fCK_PSC[Hz]
	 *
	 */

	/* TIM4 - for periodic tasks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	TIM_TimeBaseInitTypeDef TIM4_InitStructure;
	TIM4_InitStructure.TIM_Prescaler = 5999; 				//frequency prescaler - define Ts
	TIM4_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM4_InitStructure.TIM_Period = 1199; 					//auto-reload register value - define Ts
	TIM4_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM4, &TIM4_InitStructure);

	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM4, ENABLE);

	NVIC_SetPriority(TIM4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),1,0));
	NVIC_EnableIRQ(TIM4_IRQn);


	/*RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseInitTypeDef TIM2_InitStructure;
	TIM2_InitStructure.TIM_Prescaler = 5999;
	TIM2_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM2_InitStructure.TIM_Period = 11999;
	TIM2_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM2, &TIM2_InitStructure);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);

	NVIC_EnableIRQ(TIM2_IRQn);*/



	/* TIM3 is used for PWM
	 *
	 * PA6 - CH1
	 * PA7 - CH2
	 * PB0 - CH3
	 * PB1 - CH4
	 *
	*/

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructPWM;

	GPIO_InitStructPWM.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructPWM.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructPWM.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructPWM);

	//PWM frequency 5KHz
	TIM_TimeBaseInitTypeDef TIM3_InitStructure;
	TIM3_InitStructure.TIM_Prescaler = 0;
	TIM3_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM3_InitStructure.TIM_Period = PWM_period-1;
	TIM3_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM3, &TIM3_InitStructure);

	TIM_Cmd(TIM3, ENABLE);

	//PWM on CH1
	TIM_OCInitTypeDef TIM3_OCInitStruct;
	TIM3_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM3_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM3_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM3_OCInitStruct.TIM_Pulse = 0;
	TIM_OC1Init(TIM3, &TIM3_OCInitStruct);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
	TIM3->CCR1 = (uint16_t)(PWM_period*0.45);



	/* TIM2 is used for speed measurement
	 *
	 * PA3 - CH4 (speed 1)
	 * PA2 - CH3 (speed 2)
	 * */

	GPIO_InitTypeDef GPIO_InitStructInputCapture;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructInputCapture.GPIO_Pin =  GPIO_Pin_3;
	GPIO_InitStructInputCapture.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructInputCapture.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructInputCapture) ;

    TIM_ICInitTypeDef TIM2_ICInitStruct;

    TIM2_ICInitStruct.TIM_Channel = TIM_Channel_4;
    TIM2_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM2_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM2_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM2_ICInitStruct.TIM_ICFilter = 0xFF;

    TIM_ICInit(TIM2, &TIM2_ICInitStruct);
    TIM_Cmd(TIM2, ENABLE) ;

    TIM_ITConfig(TIM2, TIM_IT_CC4, ENABLE);
    NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0,0));
    NVIC_EnableIRQ(TIM2_IRQn);


}


void Button_init()
{
	GPIO_InitTypeDef GPIO_InitStructure_Buttons;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	//push buttons on PC14 and PC15; activate pull up
	GPIO_InitStructure_Buttons.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure_Buttons.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure_Buttons);

}


/* PWM value between 0-100% */
int Set_PWM_per(uint16_t PWM_val_per)
{
	if(PWM_val_per <=100)
	{
		if(PWM_val_per >= 50)
		{
			TIM3->CCR1 = (uint16_t)(PWM_period * PWM_val_per/100.0);
			return 0;
		}
		else
			return -1;
	}
	else
		return -1;
}

uint16_t Get_PWM(void)
{
	return TIM3->CCR1;
}

void Set_PWM(uint16_t PWM_val)
{
	if(PWM_val <= PWM_period)
		if(PWM_val >= 0)
			TIM3->CCR1 = PWM_val;
}


