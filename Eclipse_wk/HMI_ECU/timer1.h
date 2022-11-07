/*
 * timer.h
 *
 *  Created on: Oct 27, 2022
 *      Author: Omar Elsherif
 */

#ifndef TIMER1_H_
#define TIMER1_H_

#include"std_types.h"


/*******************************************************************************
 *                      Configuration                                          *
 *******************************************************************************/

typedef enum{
	NO_CLK,F_CPU_1, F_CPU_8,F_CPU_64, F_CPU_256, F_CPU_1024, EXTERNAL_CLK_T1_FALLING,EXTERNAL_CLK_T1_RISING
}Timer1_Prescaler;

typedef enum{
	NORMAL_MODE, CTC_MODE=0x04
}Timer1_Mode;


typedef struct {
 uint16 initial_value;
 Timer1_Mode mode;
 Timer1_Prescaler prescaler;
 uint16 compare_value; // it will be used in compare mode only.
}Timer1_ConfigType;



/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 * Function to initialize the Timer driver.
 */
void Timer1_init(const Timer1_ConfigType * TIMER1_Config);

/*
 * Description :
 * Function to disable the Timer1
 */
void Timer1_deInit(void);

/*
 * Description :
 * Function to set the Call Back function address.
 */
void Timer1_setCallBack(void(*a_ptr)(void));

#endif /* TIMER1_H_ */
