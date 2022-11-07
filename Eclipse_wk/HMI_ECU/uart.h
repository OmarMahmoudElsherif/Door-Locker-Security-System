 /******************************************************************************
 *
 * Module: UART
 *
 * File Name: uart.h
 *
 * Description: Header file for the UART AVR driver
 *
 * Author: Omar ELsherif
 *
 *******************************************************************************/

#ifndef UART_H_
#define UART_H_

#include "std_types.h"



#define TX_RISING_XCK_EDGE   0
#define TX_FALLING_XCK_EDGE  1



/*******************************************************************************
 *                      Configuration                                   *
 *******************************************************************************/


/* 	Configure Required Synchronous TX XCK edge	*/
#define SYNC_TX_XCK_EGGE  TX_RISING_XCK_EDGE



typedef uint16 UART_BaudRate;

typedef enum
{
	Asynchronous_Normal_Mode,Asynchronous_Double_Speed_Mode,Synchronous_Mode
}UART_Mode;

typedef enum
{
	MODE_5_BITS, MODE_6_BITS ,MODE_7_BITS ,MODE_8_BITS
}UART_BitData;

typedef enum
{
	PARITY_DISABLED ,EVEN_PARITY=2 ,ODD_PARITY
}UART_Parity;

typedef enum
{
	ONE_STOP_BIT,TWO_STOP_BIT
}UART_StopBit;



typedef struct
{
	UART_Mode Mode;
	UART_BitData bit_data;
	UART_Parity parity;
	UART_StopBit stop_bit;
	UART_BaudRate baud_rate;

}UART_ConfigType;



/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 * Functional responsible for Initialize the UART device by:
 * 1. Setup the Frame format like number of data bits, parity bit type and number of stop bits.
 * 2. Enable the UART.
 * 3. Setup the UART baud rate.
 */
void UART_init(const UART_ConfigType * Config_Ptr);

/*
 * Description :
 * Functional responsible for send byte to another UART device.
 */
void UART_sendByte(const uint8 data);

/*
 * Description :
 * Functional responsible for receive byte from another UART device.
 */
uint8 UART_recieveByte(void);

/*
 * Description :
 * Send the required string through UART to the other UART device.
 */
void UART_sendString(const uint8 *Str);

/*
 * Description :
 * Receive the required string until the '#' symbol through UART from the other UART device.
 */
void UART_receiveString(uint8 *Str); // Receive until #

#endif /* UART_H_ */
