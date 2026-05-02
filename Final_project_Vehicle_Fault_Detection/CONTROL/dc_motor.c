/*
 * dc_motor.c
 *
 *  Created on: Sep 30, 2025
 *      Author: Omar Mansour
 */
#include "dc_motor.h"

void DcMotor_init(void)
{
	/* Motor 1 pins */
	GPIO_setupPinDirection(DC_MOTOR1_IN1_PORT, DC_MOTOR1_IN1_PIN, PIN_OUTPUT);
	GPIO_setupPinDirection(DC_MOTOR1_IN2_PORT, DC_MOTOR1_IN2_PIN, PIN_OUTPUT);
	GPIO_writePin(DC_MOTOR1_IN1_PORT, DC_MOTOR1_IN1_PIN, LOGIC_LOW);
	GPIO_writePin(DC_MOTOR1_IN2_PORT, DC_MOTOR1_IN2_PIN, LOGIC_LOW);

	/* Motor 2 pins */
	GPIO_setupPinDirection(DC_MOTOR2_IN1_PORT, DC_MOTOR2_IN1_PIN, PIN_OUTPUT);
	GPIO_setupPinDirection(DC_MOTOR2_IN2_PORT, DC_MOTOR2_IN2_PIN, PIN_OUTPUT);
	GPIO_writePin(DC_MOTOR2_IN1_PORT, DC_MOTOR2_IN1_PIN, LOGIC_LOW);
	GPIO_writePin(DC_MOTOR2_IN2_PORT, DC_MOTOR2_IN2_PIN, LOGIC_LOW);
}

void DcMotor_Rotate(DC_MOTOR_ID motor_id, DC_MOTOR_State state)
{
	if(motor_id == MOTOR1)
	{
		switch(state)
		{
		case STOP:
			GPIO_writePin(DC_MOTOR1_IN1_PORT, DC_MOTOR1_IN1_PIN, LOGIC_LOW);
			GPIO_writePin(DC_MOTOR1_IN2_PORT, DC_MOTOR1_IN2_PIN, LOGIC_LOW);
			break;

		case CW:
			GPIO_writePin(DC_MOTOR1_IN1_PORT, DC_MOTOR1_IN1_PIN, LOGIC_HIGH);
			GPIO_writePin(DC_MOTOR1_IN2_PORT, DC_MOTOR1_IN2_PIN, LOGIC_LOW);
			break;

		case Anti_CW:
			GPIO_writePin(DC_MOTOR1_IN1_PORT, DC_MOTOR1_IN1_PIN, LOGIC_LOW);
			GPIO_writePin(DC_MOTOR1_IN2_PORT, DC_MOTOR1_IN2_PIN, LOGIC_HIGH);
			break;
		}
	}
	else if(motor_id == MOTOR2)
	{
		switch(state)
		{
		case STOP:
			GPIO_writePin(DC_MOTOR2_IN1_PORT, DC_MOTOR2_IN1_PIN, LOGIC_LOW);
			GPIO_writePin(DC_MOTOR2_IN2_PORT, DC_MOTOR2_IN2_PIN, LOGIC_LOW);
			break;

		case CW:
			GPIO_writePin(DC_MOTOR2_IN1_PORT, DC_MOTOR2_IN1_PIN, LOGIC_HIGH);
			GPIO_writePin(DC_MOTOR2_IN2_PORT, DC_MOTOR2_IN2_PIN, LOGIC_LOW);
			break;

		case Anti_CW:
			GPIO_writePin(DC_MOTOR2_IN1_PORT, DC_MOTOR2_IN1_PIN, LOGIC_LOW);
			GPIO_writePin(DC_MOTOR2_IN2_PORT, DC_MOTOR2_IN2_PIN, LOGIC_HIGH);
			break;
		}
	}
}
