/*
 * timer0.h
 *
 *  Created on: Oct 17, 2025
 *     Author: Omar Mansour
 */

#ifndef TIMER0_H_
#define TIMER0_H_
#include"gpio.h"
#include"common_macros.h"
#include<avr/io.h>
#include <avr/interrupt.h>
/*******************************************************************************
 *								 DEFINITIONS
 ********************************************************************************/


typedef enum {
	NO_CLOCK,          // 0
	F_CPU_CLOCK,       // 1
	F_CPU_8,           // 2
	F_CPU_64,          // 3
	F_CPU_256,         // 4
	F_CPU_1024,        // 5
	EXTERNAL_FALLING,  // 6
	EXTERNAL_RISING    // 7
}Timer_ClockType;


typedef enum {
	timer0, timer1, timer2
}Timer_ID_Type;

typedef enum {
	overflow =0, compare =2
}Timer_ModeType;


typedef struct
{
	Timer_ID_Type timer_ID;
	Timer_ModeType timer_mode;
	uint16 timer_InitialValue;
	uint16 timer_compare_MatchValue; /*it will be used in compare mode only*/
	Timer_ClockType timer_clock;

}Timer_ConfigType;

/*******************************************************************************
 *								 FUNCTION PROTOTYPES						    *
 ********************************************************************************/
void Timer_init(const Timer_ConfigType * Config_Ptr);
void Timer_deInit(Timer_ID_Type timer_type);
void Timer_setCallBack(void(*a_ptr)(void), Timer_ID_Type a_timer_ID );

#endif /* TIMER0_H_ */






