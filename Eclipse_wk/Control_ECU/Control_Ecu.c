/*******************************************************************************
 * Control_ECU.c
 *
 *	File Name: Control_Ecu.c
 *
 *	Description: the source file for Control_ECU application
 *
 * Author: Omar Elsherif
 *
 *******************************************************************************/




/*******************************************************************************
 *                      	Header Files	                                   *
 *******************************************************************************/

#include<avr/io.h> 				/* For I-bit*/
#include"std_types.h"			/* For uint8*/
#include"util/delay.h"			/* For delay function */
#include"uart.h"				/* For UART protocol  */
#include"string.h"				/* For strcmp() function */
#include "external_eeprom.h"	/* For External EEPROM   */
#include"twi.h"					/* For I2C Protocol */
#include"dc_motor.h"			/* For DC Motor */
#include"timer1.h"				/* For Timer 1  */
#include"buzzer.h"				/* For Buzzer   */



/*******************************************************************************
 * 																			   *
 *                      Global Variables              						   *
 *                      									                   *
 *******************************************************************************/

/*	Flag to determine if pass 2 passwords are matched */
static volatile uint8 g_passCorrectFlag=0;
/*	Flag to count number of seconds	*/
static volatile uint8 g_secondsCount=0;
/*	Flag to determine if user entered old pass correctly,
 * and wanted to change the password
 */
static volatile uint8 g_changePassFlag=0;
/*	Flag to determine if user entered the password wrong,
 * in 3 consective times
 */
static volatile uint8 g_consecWrongPass=0;



/*******************************************************************************
 *                      	  Defines                                          *
 *******************************************************************************/

#define PASSWORD_SIZE 5
#define ECU_READY 0xFF
#define MATCHED_PASSWORD 0xFE
#define UNMATCHED_PASSWORD 0xFD
#define SEND_PASSWORD 0x01
#define CONFIRM_SEND_PASSWORD 0x02
#define OPEN_DOOR 0x03
#define CHANGE_PASS 0x04



/*******************************************************************************
 * 																			   *
 *                     Function Prototypes                                	   *
 *                     													       *
 *******************************************************************************/

/* Function to receive the password from HMI_ECU*/
void receivePassword(uint8* password);
/*	This function is called every 1 second passed in timer1*/
void timer1ControlCallBack();
/* Function to check if two passwords are matched or not*/
void checkPassword(uint8*password,uint8*reEnteredPassword);
/* Function to check if entered password is matched or not matched
 * in the saved password in EEPROM.
 */
void checkPasswordInEEPROM(uint8*password);




/*******************************************************************************
 *                         main() Function                                     *
 *******************************************************************************/

int main(void)
{
	// Enable I-bit
	SREG |=(1<<7);

	/*	Initialize UART with :
	 * Asynchronous with double speed
	 * 8- bit Mode
	 * Parity is disabled
	 * 1-stop bit
	 * baud rate =9600
	 *
	 */
	UART_ConfigType UART_Config={Asynchronous_Double_Speed_Mode,MODE_8_BITS,PARITY_DISABLED,ONE_STOP_BIT,9600};
	UART_init(&UART_Config);

	/*	Initialize I2C with :
	 * 400 kbit/sec
	 * Master address = 0x01
	 */
	TWI_ConfigType TWI_Config={400000,0x01};
	TWI_init(&TWI_Config);

	/*	initialize Motor	*/
	DcMotor_Init();
	/* Initialize the buzzer */
	Buzzer_init();


	/*	Timer1 configurations to calculate 1 sec*/
	Timer1_ConfigType TIMER1_Config = {0,CTC_MODE, F_CPU_256, 31250};
	Timer1_init(&TIMER1_Config);
	/*	Set the callback function of timer1*/
	Timer1_setCallBack(timer1ControlCallBack);




	/* looping until HMI send ECU_READY signal */
	while (UART_recieveByte() != ECU_READY);
	/* sending to HMI_ECU ECU_READY signal */
	UART_sendByte(ECU_READY);

	/* 5-digit Password + null +'#' when we send using UART */
	uint8 password[PASSWORD_SIZE+2];
	uint8 reEnteredPassword[PASSWORD_SIZE+2];

	while(1)
	{
		/* receive the password from HMI_ECU */
		receivePassword(password);
		/* receive the re entered password from HMI_ECU */
		receivePassword(reEnteredPassword);
		/* Check the two passwords */
		checkPassword(password,reEnteredPassword);

		/* if password matched we are in inner menu
		 *  '+' : Open Door
		 * '-' : change Pass
		 */
		if(g_passCorrectFlag==1)
		{

			while(1)
			{
				/* receive the password from HMI_ECU */
				receivePassword(reEnteredPassword);
				/* check that received password with the one saved in EEPROM */
				checkPasswordInEEPROM(reEnteredPassword);
				/* if user wants to change password and entered the old one correctly*/
				if(g_changePassFlag==1)
				{
					/* Clear the change password flag	*/
					g_changePassFlag=0;
					/* Break to outer while(1) which has outer menu	*/
					break;
				}
				/*	if user entered the password wrong 3 consecutive times	*/
				if(g_consecWrongPass==3)
				{
					/* Clear the consecutive password counter	*/
					g_consecWrongPass=0;
					/* Clear the seconds counter to start count from beginning	*/
					g_secondsCount=0;
					/*	Activate Buzzer */
					Buzzer_on();
					/* Wait until 60 seconds are passed*/
					while(g_secondsCount<60);
					/*	De-activate the buzzer */
					Buzzer_off();
				}

			}/* End of inner while(1) */


		}/* End of if(g_passCorrectFlag==1) */


	}/* End of while(1)	*/



	return 1;

}	/* End of main() function*/





/*******************************************************************************
 * 																			   *
 *                     Function Definitions									   *
 *                     					                                       *
 *******************************************************************************/


/* Function to receive the password from HMI_ECU*/
void receivePassword(uint8* password)
{
	/* Loop untill the HMI_ECU is ready to send the password*/
	while (UART_recieveByte() != SEND_PASSWORD);
	UART_sendByte(CONFIRM_SEND_PASSWORD);
	/* Receive the password from HMI_ECU	*/
	UART_receiveString(password);
}

/*	This function is called every 1 second passed in timer1*/
void timer1ControlCallBack()
{
	/* Increment the seconds counter */
	g_secondsCount++;
}

/* Function to check if two passwords are matched or not*/
void checkPassword(uint8*password,uint8*reEnteredPassword)
{
	/*if two passwords are matched , strcmp() = 0*/
	if(!strcmp(password,reEnteredPassword))
	{
		/* Set the Passwords correct flag*/
		g_passCorrectFlag=1;

		/* Write Password in the external EEPROM */
		uint8 i=0;
		for(i=0;i<=PASSWORD_SIZE;i++)
		{
			/*	Since EEPROM each location inside it has 1 byte,
			 * so next location we increment address by 1 */
			EEPROM_writeByte(0x0311+i, password[i]);
			/*	Must make 10 ms delay between each write/read operation in EEPROM*/
			_delay_ms(10);
		}
		/*	Send to HMI_ECU that Control_ECU is ready to send	*/
		UART_sendByte(SEND_PASSWORD);
		while ( UART_recieveByte() != CONFIRM_SEND_PASSWORD);
		/*	Send to HMI_ECU that password is matched */
		UART_sendByte(MATCHED_PASSWORD);
	}
	/*if two passwords are NOT Matched */
	else
	{
		/* Clear the Passwords correct flag*/
		g_passCorrectFlag=0;
		/*	Send to HMI_ECU that Control_ECU is ready to send	*/
		UART_sendByte(SEND_PASSWORD);
		while ( UART_recieveByte() != CONFIRM_SEND_PASSWORD);
		/*	Send to HMI_ECU that password is NOT Matched */
		UART_sendByte(UNMATCHED_PASSWORD);
	}

}

/* Function to check if entered password is matched or not matched
 * in the saved password in EEPROM.
 */
void checkPasswordInEEPROM(uint8*password)
{
	uint8 i=0;
	/* Make array of unsigned character to store inside it
	 * the password saved in EEPROM.
	 */
	uint8 savedPassword[PASSWORD_SIZE+2];
	/*	Read saved password from EEPROM*/
	for(i=0;i<=PASSWORD_SIZE;i++)
	{
		/*	Since EEPROM each location inside it has 1 byte,
		 * so next location we increment address by 1*/
		EEPROM_readByte(0x0311+i, &savedPassword[i]);
		/*	Must make 10 ms delay between each write/read operation in EEPROM*/
		_delay_ms(10);
	}

	/*If password in EEPROM is Matched with password entered by user*/
	if(!strcmp(password,savedPassword))
	{
		/*	Send to HMI_ECU that Control_ECU is ready to send	*/
		UART_sendByte(SEND_PASSWORD);
		while ( UART_recieveByte() != CONFIRM_SEND_PASSWORD);
		/*	Send to HMI_ECU that password is matched */
		UART_sendByte(MATCHED_PASSWORD);

		/*	Variable to determine if HMI_ECU wants to open door or change the password*/
		uint8 HMIStatus;
		HMIStatus=UART_recieveByte();

		/* If HMI_ECU wants to Open Door */
		if(HMIStatus==OPEN_DOOR)
		{
			/* Clear the seconds counter to start counting from beginning*/
			g_secondsCount=0;
			/*	Rotate Motor Clockwise at 50 %	*/
			DcMotor_Rotate(Clockwise,50);
			/*	wait till 15 seconds are passed	*/
			while(g_secondsCount<15);
			/* Stop the motor */
			DcMotor_Rotate(Stop,0);
			/*	wait till another 3 seconds are passed	*/
			while(g_secondsCount<18);
			/*	Rotate Motor Anti Clockwise at 50 %	*/
			DcMotor_Rotate(Anti_Clockwise,50);
			/*	wait till another 15 seconds are passed	*/
			while(g_secondsCount<33);
			/* Stop the motor */
			DcMotor_Rotate(Stop,0);
		}

		/* If HMI_ECU wants to Change Password */
		if(HMIStatus==CHANGE_PASS)
		{
			/*	Set the change password flag */
			g_changePassFlag=1;
		}
		/* Since the two passwords are matched then we
		 * clear consecutive wrong password counter */
		g_consecWrongPass=0;

	}/* End of if(!strcmp(password,savedPassword)) */

	/*If password in EEPROM is NOT Matched with password entered by user*/
	else
	{
		/* increment the consecutive wrong password counter */
		g_consecWrongPass++;
		/* clear the correct password flag,since two passwords are NOT matched*/
		g_passCorrectFlag=0;
		/*	Send to HMI_ECU that Control_ECU is ready to send	*/
		UART_sendByte(SEND_PASSWORD);
		while ( UART_recieveByte() != CONFIRM_SEND_PASSWORD);
		/*	Send to HMI_ECU that password is matched */
		UART_sendByte(UNMATCHED_PASSWORD);
	}

}



