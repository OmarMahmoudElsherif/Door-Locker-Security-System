/*******************************************************************************
 * Human_Machne_Interface.c
 *
 *	File Name: Human_Machne_Interface.c
 *
 *	Description: the source file for HMI_ECU application
 *
 * Author: Omar Elsherif
 *
 *******************************************************************************/



/*******************************************************************************
 *                      	Header Files	                                   *
 *******************************************************************************/


#include<avr/io.h> 		/* For I-bit*/
#include"std_types.h"	/* For uint8*/
#include"lcd.h"			/* For LCD */
#include"keypad.h"		/* For Keypad */
#include"util/delay.h"	/* For delay function */
#include"uart.h"		/* For UART protocol */
#include"timer1.h"		/* For Timer 1 */


/*******************************************************************************
 * 																			   *
 *                      Global Variables              						   *
 *                      									                   *
 *******************************************************************************/

/*	Flag to count number of seconds	*/
static volatile uint8 g_secondsCount=0;
/*	Flag to determine if user entered the password wrong,
 * in 3 consective times
 */
static volatile uint8 g_consectiveWrongPasswords=0;
/*	Flag to determine if user entered old pass correctly,
 * and wanted to change the password
 */
static volatile uint8 g_changepassFlag=0;



/*******************************************************************************
 *                      	  Defines                                          *
 *******************************************************************************/

#define KEYPAD_ENTER_CHARACTER '='
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



/*	This function is called every 1 second passed in timer1*/
void timer1HMICallback(void);
/*	Function to get password from user and
 * send it to the Contol_ECU*/
void getAndSendPassword(void);
/*	Function to Open the Door	*/
void openDoor(void);
/*	Function to change the password if user
 *  wanted to change the password and entered
 *  the old password correct.
 */
void changePassword();




/*******************************************************************************
 *                         main() Function                                     *
 *******************************************************************************/


int main(void)
{
	// Enable I-bit
	SREG |=(1<<7);
	/*	Initialize the LCD*/
	LCD_init();

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

	/*	Timer1 configurations to calculate 1 second*/
	Timer1_ConfigType TIMER1_Config = {0,CTC_MODE, F_CPU_256, 31250};
	Timer1_init(&TIMER1_Config);

	/*	Set the callback function of timer1*/
	Timer1_setCallBack(timer1HMICallback);

	/* sending to CONTROL_ECU ECU_READY signal */
	UART_sendByte(ECU_READY);
	/* looping until CONTROL_ECU send ECU_READY signal */
	while ( UART_recieveByte() != ECU_READY);

	/* variable to know if password match or not match*/
	uint8 passStatus;

	while(1)
	{
		/*	Clear LCD screen*/
		LCD_clearScreen();
		LCD_displayString("Plz Enter Pass:");
		/*	Move LCD cursor to second line*/
		LCD_moveCursor(1, 0);
		/* Get password from user */
		getAndSendPassword();
		/*	Clear LCD screen*/
		LCD_clearScreen();
		/*	Move LCD cursor to First line*/
		LCD_moveCursor(0, 0);
		LCD_displayString("Plz Re-Enter the:");
		/*	Move LCD cursor to second line*/
		LCD_moveCursor(1, 0);
		LCD_displayString("same pass:");
		/* Get password from user */
		getAndSendPassword();



		/* looping until CONTROL_ECU is ready to receive the password */
		while (UART_recieveByte() != SEND_PASSWORD);
		/* sending to CONTROL_ECU  signal to let it know that HMI_ECU will send the password*/
		UART_sendByte(CONFIRM_SEND_PASSWORD);

		/*	Get the password status from CONTROL_ECU to identify if the
		 * two passwords are matched or not	*/
		passStatus = UART_recieveByte();

		LCD_clearScreen();

		/* if passwords were matched*/
		if(passStatus==MATCHED_PASSWORD)
		{

			while(1)
			{
				/* if user entered password wrong 3 consecutive times */
				if(g_consectiveWrongPasswords==3)
				{
					/* clear the counter of consecutive wrong passwords*/
					g_consectiveWrongPasswords=0;
					/* clear second count	*/
					g_secondsCount=0;
					LCD_clearScreen();
					LCD_displayString("ERROR !!!");
					/* loop till there are 60 seconds passed */
					while(g_secondsCount<60);

				}
				/* if passwords match and not wrong in 3 consecutive times , display :
				 * '+' : Open Door
				 * '-' : change Pass
				 */
				LCD_clearScreen();
				LCD_displayString("+ : Open Door");
				LCD_moveCursor(1, 0);
				LCD_displayString("- : Change Pass");

				/* if user wanted to open the door '+' */
				if(KEYPAD_getPressedKey()=='+')
				{
					_delay_ms(200);
					/* Open the door	*/
					openDoor();
				}
				/* if user wanted to change the pass '-' */
				else if(KEYPAD_getPressedKey()=='-')
				{
					_delay_ms(200);
					/* Change the password	*/
					changePassword();
				}

				/* If user wanted to change the password and entered the old
				 * password correctly */
				if(g_changepassFlag==1)
				{
					/*	Clear the change pass flag 	*/
					g_changepassFlag=0;
					/* break and go to outer menu to get new	*/
					break;
				}


			} /* End of inner while(1) */

		}/*	End of if(passStatus==MATCHED_PASSWORD) */


		/* if the 2 entered passwords were not matched	*/
		else if(passStatus==UNMATCHED_PASSWORD)
		{
			LCD_displayString("NOT MATCHED ");
			_delay_ms(2000);
		}

	} /* End Of While(1)*/


	return 1;
} /* End of main() function */




/*******************************************************************************
 * 																			   *
 *                     Function Definitions									   *
 *                     					                                       *
 *******************************************************************************/





/*	This function is called every 1 second passed in timer1*/
void timer1HMICallback(void)
{
	/* increments the seconds counter*/
	g_secondsCount++;
}



/*	Function to get password from user and
 * send it to the Contol_ECU*/
void getAndSendPassword(void)
{
	/*	variable to loop over password 	*/
	uint8 passwordSize=0;
	/* 5-digit Password + null +'#' when we send using UART */
	uint8 password[PASSWORD_SIZE+2];
	/* Loop to get password from user via keypad*/
	for(passwordSize=0;passwordSize< PASSWORD_SIZE;passwordSize++)
	{
		/* Store password	*/
		password[passwordSize] = KEYPAD_getPressedKey();
		LCD_displayCharacter('*');
		//LCD_intgerToString(password[passwordSize]);
		_delay_ms(500);
	}
	/*	Put null after filling the string */
	//password[++passwordSize]='\0';
	password[passwordSize]='#';
	password[++passwordSize]='\0';


	/* As long as '=' is not pressed we wait */
	while(KEYPAD_getPressedKey() != KEYPAD_ENTER_CHARACTER);
	_delay_ms(100);

	/* Send signal to Control_ECU to let him know that HMI_ECU will send password */
	UART_sendByte(SEND_PASSWORD);
	while ( UART_recieveByte() != CONFIRM_SEND_PASSWORD);
	/*Send Each digit password to Control_ECU*/
	UART_sendString(password);
}

/*	Function to Open the Door	*/
void openDoor(void)
{
	/*	Variable to determine status of entered password,
	 * whether it is matched or not matched
	 */
	uint8 passwordStatus;

	LCD_clearScreen();
	LCD_displayString("Plz Enter Pass:");
	LCD_moveCursor(1, 0);
	/* Calling function getAndSendPassword() ,
	 * that get the pass from user and send it to Control_ECU
	 */
	getAndSendPassword();


	/*	Wait till the Control_ECU send a signal to let HMI_ECU know that it will
	 * receive status of password from Control_ECU
	 */
	while ( UART_recieveByte() != SEND_PASSWORD);
	UART_sendByte(CONFIRM_SEND_PASSWORD);
	/*	Receive the password status, whether entered pass in matched or not*/
	passwordStatus=UART_recieveByte();
	/* Send a signal to Control_ECU to let him know that we are in open Door function */
	UART_sendByte(OPEN_DOOR);

	/*	If the Two password matched */
	if(passwordStatus==MATCHED_PASSWORD)
	{
		/*	Clear the consecutive wrong password counter */
		g_consectiveWrongPasswords=0;
		/*	Clear the seconds counter */
		g_secondsCount=0;
		LCD_clearScreen();
		/*	Display Door Unlocking */
		LCD_displayString("Door Unlocking");
		/*	Wait till 15 seconds were passed */
		while(g_secondsCount<15);
		LCD_clearScreen();
		/*	Display Door Opened */
		LCD_displayString("Door Opened");
		/*	Wait till another 3 seconds were passed */
		while(g_secondsCount<=18);
		LCD_clearScreen();
		/*	Display Door Locking */
		LCD_displayString("Door Locking");
		/*	Wait till another 15 seconds were passed */
		while(g_secondsCount<33);
	}

	/*	If the Two passwords are NOT matched */
	else if(passwordStatus==UNMATCHED_PASSWORD)
	{
		/*	Increment the consecutive wrong password counter */
		g_consectiveWrongPasswords++;
		/* if user entered consecutive wrong pass less than 3 times*/
		if(g_consectiveWrongPasswords<3)
		{
			LCD_clearScreen();
			/* Display "Wrong Pass:" and display the number of wrong password times*/
			LCD_displayString("Wrong Pass: ");
			LCD_intgerToString(g_consectiveWrongPasswords);
			_delay_ms(2000);
			/* If two password are not matched get another password from user	*/
			openDoor();
		}
	}
}

/*	Function to change the password if user
 *  wanted to change the password and entered
 *  the old password correct.
 */
void changePassword()
{
	/*	Variable to determine status of entered password,
	 * whether it is matched or not matched
	 */
	uint8 passwordStatus;

	LCD_clearScreen();
	LCD_displayString("Plz Enter Pass: ");
	LCD_moveCursor(1, 0);
	/* Calling function getAndSendPassword() ,
	 * that get the pass from user and send it to Control_ECU
	 */
	getAndSendPassword();


	/*	Wait till the Control_ECU send a signal to let HMI_ECU know that it will
	 * receive status of password from Control_ECU
	 */
	while ( UART_recieveByte() != SEND_PASSWORD);
	UART_sendByte(CONFIRM_SEND_PASSWORD);
	/*	Receive the password status, whether entered pass in matched or not*/
	passwordStatus=UART_recieveByte();
	/* Send a signal to Control_ECU to let him know that we are in Change pass function */
	UART_sendByte(CHANGE_PASS);

	/*	If the Two passwords matched */
	if(passwordStatus==MATCHED_PASSWORD)
	{
		/*	Clear the consecutive wrong password counter */
		g_consectiveWrongPasswords=0;
		/* Set the change password flag */
		g_changepassFlag=1;
		LCD_clearScreen();
		LCD_displayString("Change Password");
		LCD_moveCursor(1, 0);
		LCD_displayString("Confirmed");
		_delay_ms(2000);
	}
	/*	If the Two passwords are NOT matched */
	else if(passwordStatus==UNMATCHED_PASSWORD)
	{
		/* Clear the change password flag */
		g_changepassFlag=0;
		/*	Increment the consecutive wrong password counter */
		g_consectiveWrongPasswords++;
		/* if user entered consecutive wrong pass less than 3 times*/
		if(g_consectiveWrongPasswords<3)
		{
			LCD_clearScreen();
			/* Display "Wrong Pass:" and display the number of wrong password times*/
			LCD_displayString("Wrong Pass: ");
			LCD_intgerToString(g_consectiveWrongPasswords);
			_delay_ms(2000);
			/* If two password are not matched get another password from user	*/
			changePassword();
		}
	}
}



