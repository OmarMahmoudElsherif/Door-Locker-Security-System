/*
 * dc_motor.h
 *
 *  Created on: 4 Oct 2022
 *      Author: Omar Elsherif
 */

#ifndef DC_MOTOR_H_
#define DC_MOTOR_H_

#include "gpio.h"
#include "PWM_Timer0.h"
/*******************************************************************************
 *                      DC Motor Configurations                                *
 *******************************************************************************/
typedef enum{
	Stop,Clockwise,Anti_Clockwise
}DcMotor_State;


/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define Motor1_PORT_ID		PORTB_ID
#define Motor1_INPUT_PIN1	PIN0_ID
#define Motor1_INPUT_PIN2   PIN1_ID




/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 * The Function responsible for setup the direction for the two motor pins through the GPIO driver.
 * Stop at the DC-Motor at the beginning through the GPIO driver.
 */
void DcMotor_Init(void);

/*
 * Description :
 * The function responsible for rotate the DC Motor CW/ or A-CW or stop the motor based on the state input state value.
 * Send the required duty cycle to the PWM driver based on the required speed value.

 */
void DcMotor_Rotate(DcMotor_State state,uint8 speed);



#endif /* DC_MOTOR_H_ */
