/*
 * pwm.c
 *
 *  Created on: Sep 28, 2025
 *      Author: Omar Mansour
 */

#include "pwm.h"
#include "gpio.h"

void Timer0_PWM_Init(const PWM_ConfigType * config_Ptr ) // frequency will be set in the main
{

	TCNT0 = 0; // Set Timer Initial Value to 0

	//pin where the PWM signal is generated from MC
	GPIO_setupPinDirection(config_Ptr->PWM_Port, config_Ptr->PWM_Pin, PIN_OUTPUT);


	//set duty cycle as a percentage of the max speed
	OCR0  = (uint8)( ( uint16 ) ((config_Ptr->PWM_Speed)*PWM_MAX_SPEED)/100);


	/* Configure timer control register
	 * 1. Fast PWM mode FOC0=0
	 * 2. Fast PWM Mode WGM01=1 & WGM00=1
	 */
	TCCR0 = (1<<WGM00) | (1<<WGM01);

	//set mode inverting or no-inverting

	if(config_Ptr->mode == inverting)
	{
		TCCR0 |= (1<<COM01) |(1<<COM00);
	}
	else if(config_Ptr->mode == non_inverting)
	{
		TCCR0 |= (1<<COM01);
	}

	TCCR0 |= config_Ptr->pre_scaler; // clock configuration

}









