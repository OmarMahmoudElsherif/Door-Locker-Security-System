/*
 * timer.c
 *
 *  Created on: Oct 27, 2022
 *      Author: Omar Elsherif
 */

#include <avr/interrupt.h> /* For Timer ISR */
#include <avr/io.h>	/* For Register names */
#include "timer1.h"

/*******************************************************************************
 *                      Global Variables                                  *
 *******************************************************************************/

static volatile void (*g_callBackPtr)(void) = NULL_PTR;


/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/


/*
 * Description :
 * Function to initialize the Timer driver.
 */
void Timer1_init(const Timer1_ConfigType * TIMER1_Config)
{

	/* FOC1A,FOC1B  : are only active when specifying non-pwm mode */
	TCCR1A = (1<<FOC1A) | (1<<FOC1B);
	/* Select the modes WGM11,WGM10 , {Normal Mode or Compare Mode} */
	TCCR1A = (TCCR1A & 0xFC) | ((TIMER1_Config->mode)&0x03);
	/* configure WGM13, WGM12 */
	TCCR1B = (TCCR1B & 0xE7) | (((TIMER1_Config->mode) & 0x0C)<<1);

	/* Configure prescaler , CS12, CS11, CS10*/
	TCCR1B = (TCCR1B & 0xF8) | ((TIMER1_Config->prescaler)&0x07);

	/* Value of Timer initially */
	TCNT1 = TIMER1_Config->initial_value;

	if(TIMER1_Config->mode == CTC_MODE)
	{
		/* Compare value is put in OCR1A register*/
		OCR1A = TIMER1_Config->compare_value;

		/* Enable the Output Compare A Match Interrupt Enable */
		TIMSK = (1<<OCIE1A);
	}
	else if (TIMER1_Config->mode == NORMAL_MODE)
	{
		/* Enable the Output Compare A Match Interrupt Enable */
		TIMSK = (1<<TOIE1);
	}
}

/*
 * Description :
 * Function to disable the Timer1
 */
void Timer1_deInit(void)
{
	/* deInit the Whole Timer1*/
	TCCR1A = 0;
	TCNT1 = 0;
	OCR1A = 0;
	TIMSK = 0;
}


/*
 * Description :
 * Function to set the Call Back function address.
 */
void Timer1_setCallBack(void(*a_ptr)(void))
{
	/* Save the address of the Call back function in a global variable */
	g_callBackPtr = a_ptr;
}


/*******************************************************************************
 *                      		ISRs 		                                   *
 *******************************************************************************/

ISR(TIMER1_OVF_vect)
{
	if(g_callBackPtr != NULL_PTR)
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_callBackPtr)(); /* another method to call the function using pointer to function g_callBackPtr(); */
	}
}

ISR(TIMER1_COMPA_vect)
{
	if(g_callBackPtr != NULL_PTR)
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_callBackPtr)(); /* another method to call the function using pointer to function g_callBackPtr(); */
	}

}

