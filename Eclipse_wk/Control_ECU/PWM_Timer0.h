/*
 * PWM_Timer0.h
 *
 *  Created on: 4 Oct 2022
 *      Author: Omar Elsherif
 */

#ifndef PWM_TIMER0_H_
#define PWM_TIMER0_H_
#include <avr/io.h>
#include "std_types.h"


/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define MAX_DUTY_CYCLE_PERCENTAGE 100
#define MAX_TIMER0_VALUE     255



/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 * The function responsible for trigger the Timer0 with the PWM Mode
 * Setup the PWM mode with Non-Inverting.
 * Setup the prescaler with F_CPU/8.
 * Setup the compare value based on the required input duty cycle
 * Setup the direction for OC0 as output pin through the GPIO driver
 * The generated PWM signal frequency will be 500Hz to control the DC Motor speed.
 */

void PWM_Timer0_Start(uint8 duty_cycle);



#endif /* PWM_TIMER0_H_ */
