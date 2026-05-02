/*
 * dc_motor.h
 *
 *  Created on: Sep 30, 2025
 *      Author: Omar Mansour
 */

#ifndef DC_MOTOR_H_
#define DC_MOTOR_H_
#include"gpio.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
/* Motor 1 (Window 1) */
#define DC_MOTOR1_IN1_PORT   PORTB_ID
#define DC_MOTOR1_IN1_PIN    PIN0_ID
#define DC_MOTOR1_IN2_PORT   PORTB_ID
#define DC_MOTOR1_IN2_PIN    PIN1_ID

/* Motor 2 (Window 2) */
#define DC_MOTOR2_IN1_PORT   PORTB_ID
#define DC_MOTOR2_IN1_PIN    PIN6_ID
#define DC_MOTOR2_IN2_PORT   PORTB_ID
#define DC_MOTOR2_IN2_PIN    PIN7_ID

typedef enum{
	CW, Anti_CW, STOP
}DC_MOTOR_State;

typedef enum{
	MOTOR1, MOTOR2
}DC_MOTOR_ID;

void DcMotor_init(void);
void DcMotor_Rotate(DC_MOTOR_ID motor_id, DC_MOTOR_State state);

#endif /* DC_MOTOR_H_ */
