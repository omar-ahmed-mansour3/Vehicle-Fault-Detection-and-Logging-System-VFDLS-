/****************************************************************************************
 * FILE NAME: icu.c
 *
 * DESCRIPTION: ICU driver Header file
 *
 * Author: Alaa Medhat
 *
*****************************************************************************************/


#ifndef ICU_H_
#define ICU_H_

/****************************************Includes****************************************/

#include "std_types.h"

/***************************************Definitions**************************************/

/*
 * Defining the input capture pin and port in the ATMEGA16 micro-controler.
 */
#define ICU_PORT             PORTD_ID
#define ICU_PIN              PIN6_ID

/*************************************Types Declaration***********************************/

/*
 * enum name:
 * ICU_Clock
 * Definition:
 * Represents the frequency Timer1 can operate on.
 */
typedef enum
{
	NO_CLOCK, FCPU_CLOCK, FCPU_8_CLOCK, FCPU_64_CLOCK, FCPU_256_CLOCK,\
	FCPU_1024_CLOCK, EXTERNAL_CLOCK_ON_T1_PIN_ON_FALLING_EDGE,\
	EXTERNAL_CLOCK_ON_T1_PIN_ON_RISING_EDGE
}ICU_Clock;

/*
 * enum name:
 * ICU_EdgeType
 * Definition:
 * Represents the edges ICU can detect on input capture pin (ICP1 pin).
 */
typedef enum
{
	FALLING_EDGE, RISING_EDGE
}ICU_EdgeType;

/*
 * struct name:
 * ICU_ConfigType
 * Definition:
 * Represents the Dynamic configurations that we may want to change in runtime:
 * 1- Clock/frequency Timer1 operates on.
 * 2- Edge required to detect on input capture pin (ICP1 pin).
 */
typedef struct
{
	ICU_Clock clock;
	ICU_EdgeType edge;
}ICU_ConfigType;


/**********************************Functions Prototypes**********************************/

/*
 * Description:
 * Initialize the ICU
 * 1- Initialize Timer 1 registers.
 * 2- set the ICP pin as input, Operate Timer1 in normal mode,
 * 3- configure the Timer1 clock, and the capture event that ICP1 pin is supposed to detect.
 * 4- Enable the Input Capture interrupt.
 * Arguments:
 * const ICU_ConfigType *config_Ptr: the configuration structure of the ICU module.
 * It contain the clock in which Timer1 will operate and the capture event
 * that ICP1 pin is supposed to detect.
 */
void ICU_init(const ICU_ConfigType *a_config_Ptr);

/*
 * Description:
 * Assign the call back function address passed from the Application layer
 * into the global pointer to call back function.
 * Argument:
 * void(*a_callBackFunction_ptr)(void): call back function address passed from the Application layer
 * to be executed in the ICU ISR.
 */
void ICU_setCallBack(void(*a_callBackFunction_ptr)(void));

/*
 * Description:
 * Set the edge on the Input Capture Pin (ICP1) that is used to trigger a capture event.
 * Argument:
 * const ICU_EdgeType a_edgeType: edge on the Input Capture Pin (ICP1) that trigger a capture event.
 */
void ICU_setEdgeDetectionType(const ICU_EdgeType a_edgeType);

/*
 * Description:
 * Returns the value of the ICR1
 * The value of the ICR1 is updated when the required event is captured.
 */
uint16 ICU_getInputCaptureValue(void);

/*
 * Description:
 * Clear the value of the TCNT1 to start counting from zero.
 */
void ICU_clearTimerValue(void);

/*
 * Description:
 * 1- This function clear all the Timer1 registers used by ICU.
 * 2- Stop the Timer1.
 * 3- Disable the input capture interrupt.
 */
void ICU_deinit(void);

#endif /* ICU_H_ */
