/*
 * stm32f103_misc.h
 *
 *  Created on: Sep 23, 2018
 *      Author: rgrbic
 */

#ifndef MISC_STM32F103_STM32F103_MISC_H_
#define MISC_STM32F103_STM32F103_MISC_H_

#include "stm32f10x.h"

#define STRING_BUF_SIZE		50	//Maximum size of the string
#define DECIMAL_PART_SIZE	4
#define CONSTANT_PI	3.141592654

char string_buffer[STRING_BUF_SIZE];

void Systick_init(void);
void Delay_ms(uint32_t delayMs);
void Output_setup(void);
void Timer_setup(void);
void Button_init();
int Set_PWM_per(uint16_t PWM_val_per);
uint16_t Get_PWM(void);
void Set_PWM(uint16_t PWM_val);


#endif /* MISC_STM32F103_STM32F103_MISC_H_ */
