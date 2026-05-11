/*
 * pwm.h
 *
 *  Created on: Sep 28, 2025
 *
 *  Description: Header file for the AVR TIMER 0 PWM driver
 *      Author: Omar Mansour
 */

#ifndef PWM_H_
#define PWM_H_
#include <avr/io.h>
#include"gpio.h"
#include "std_types.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define PWM_MAX_SPEED 255

/***********************************************************************
 * 								type Declarations 					   *
***********************************************************************/

typedef enum{
	inverting, non_inverting
}PWM_Mode;

typedef enum{
	NO_CLOCK,F_CPU_CLOCK,F_CPU_8,F_CPU_64,F_CPU_256,F_CPU_1024

}PWM_PreScaler;



typedef struct{
	uint8 PWM_Port;
	uint8 PWM_Pin;
	uint8 PWM_Speed;
	PWM_Mode mode; // inverting or non-inverting
	PWM_PreScaler pre_scaler;

}PWM_ConfigType;

void Timer0_PWM_Init(const PWM_ConfigType * config_Ptr );






#endif /* PWM_H_ */
















