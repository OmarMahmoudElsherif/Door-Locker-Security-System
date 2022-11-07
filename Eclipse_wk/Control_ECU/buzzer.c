/*
 * buzzer.c
 *
 *  Created on: Oct 26, 2022
 *      Author: Omar Elsherif
 */


#include"buzzer.h"
#include"gpio.h"
#include"std_types.h"

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/


/*
 * Description :
 * 1. Setup the direction for the buzzer pin as output pin through the GPIO driver.
 * 2. Turn off the buzzer through the GPIO.
 */
void Buzzer_init(void)
{
	/*	Setup Pin of buzzer as Output*/
	GPIO_setupPinDirection(BUZZER_PORT_ID, BUZZER_PIN_ID, PIN_OUTPUT);

	/*	Initially Turn Off the Buzzer */
	GPIO_writePin(BUZZER_PORT_ID, BUZZER_PIN_ID, LOGIC_LOW);
}

/*
 * Description :
 * Function to enable the Buzzer through the GPIO
 */
void Buzzer_on(void)
{
	/*	Turn On the Buzzer */
	GPIO_writePin(BUZZER_PORT_ID, BUZZER_PIN_ID, LOGIC_HIGH);
}

/*
 * Description :
 * Function to disable the Buzzer through the GPIO
 */
void Buzzer_off(void)
{
	/*	Turn Off the Buzzer */
	GPIO_writePin(BUZZER_PORT_ID, BUZZER_PIN_ID, LOGIC_LOW);
}
