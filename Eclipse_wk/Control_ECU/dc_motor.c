/*
 * dc_motor.c
 *
 *  Created on: 4 Oct 2022
 *      Author: Omar Elsheriif
 */

#include "dc_motor.h"


void DcMotor_Init(void)
{
	/* Setting up motor1 o/p pins*/
	GPIO_setupPinDirection(Motor1_PORT_ID, Motor1_INPUT_PIN1, PIN_OUTPUT);
	GPIO_setupPinDirection(Motor1_PORT_ID, Motor1_INPUT_PIN2, PIN_OUTPUT);

	/* Initially Motor is Stopped*/
	GPIO_writePin(Motor1_PORT_ID,Motor1_INPUT_PIN1,Stop);
	GPIO_writePin(Motor1_PORT_ID,Motor1_INPUT_PIN2,Stop);

}


void DcMotor_Rotate(DcMotor_State state,uint8 speed)
{
	/* change the state of the motor according to input state given */
	GPIO_writePin(Motor1_PORT_ID,Motor1_INPUT_PIN1,(state&0x01));
	GPIO_writePin(Motor1_PORT_ID,Motor1_INPUT_PIN2,((state&0x02)>>1));

	/* generate duty cycle of speed value (0-->100) to get required motor speed */
	PWM_Timer0_Start(speed);

}
