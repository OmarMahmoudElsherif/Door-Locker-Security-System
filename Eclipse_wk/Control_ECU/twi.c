 /******************************************************************************
 *
 * Module: TWI(I2C)
 *
 * File Name: twi.h
 *
 * Description: Source file for the TWI(I2C) AVR driver
 *
 *******************************************************************************/
 
#include "twi.h"
#include "common_macros.h"
#include <avr/io.h>



void TWI_init(const TWI_ConfigType * Config_Ptr)
{
    /* Bit Rate: 400.000 kbps using zero pre-scaler TWPS=00 and F_CPU=8Mhz */
	uint8 Prescaler;
	sint8 Twbr_value=0;
	uint8 four_pow_prescaler=1;
	for(Prescaler=0;Prescaler<4;Prescaler++)
	{
		Twbr_value = F_CPU/((Config_Ptr->bit_rate)*2*four_pow_prescaler)  - 16/(2*four_pow_prescaler);

		if(Twbr_value>0)	// if positive number, break and put that value in TWBR register
			break;

		four_pow_prescaler *= 4;
	}

    TWBR = Twbr_value;
	TWSR = Prescaler;
	
    /* Two Wire Bus address my address if any master device want to call me: 0x1 (used in case this MC is a slave device)
       General Call Recognition: Off */
  //  TWAR = 0b00000010; // my address = 0x01 :)

	TWAR = (Config_Ptr->address)<<1 ;
	
    TWCR = (1<<TWEN); /* enable TWI */
}

void TWI_start(void)
{
    /* 
	 * Clear the TWINT flag before sending the start bit TWINT=1
	 * send the start bit by TWSTA=1
	 * Enable TWI Module TWEN=1 
	 */
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    /*	Note, we did here 1<<TWEN since we makdeTWCR= ,  why not =|
     * -> because every time we need to reset flags of TWCR registers
     */
    
    /* Wait for TWINT flag set in TWCR Register (start bit is send successfully) */
    while(BIT_IS_CLEAR(TWCR,TWINT));
}

void TWI_stop(void)
{
    /* 
	 * Clear the TWINT flag before sending the stop bit TWINT=1
	 * send the stop bit by TWSTO=1
	 * Enable TWI Module TWEN=1 
	 */
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void TWI_writeByte(uint8 data)
{
    /* Put data On TWI data Register */
    TWDR = data;
    /* 
	 * Clear the TWINT flag before sending the data TWINT=1
	 * Enable TWI Module TWEN=1 
	 */ 
    TWCR = (1 << TWINT) | (1 << TWEN);
    /* Wait for TWINT flag set in TWCR Register(data is send successfully) */
    while(BIT_IS_CLEAR(TWCR,TWINT));
}

uint8 TWI_readByteWithACK(void)
{
	/* 
	 * Clear the TWINT flag before reading the data TWINT=1
	 * Enable sending ACK after reading or receiving data TWEA=1
	 * Enable TWI Module TWEN=1 
	 */ 
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    /* Wait for TWINT flag set in TWCR Register (data received successfully) */
    while(BIT_IS_CLEAR(TWCR,TWINT));
    /* Read Data */
    return TWDR;
}

uint8 TWI_readByteWithNACK(void)
{
	/* 
	 * Clear the TWINT flag before reading the data TWINT=1
	 * Enable TWI Module TWEN=1 
	 */
    TWCR = (1 << TWINT) | (1 << TWEN);
    /* Wait for TWINT flag set in TWCR Register (data received successfully) */
    while(BIT_IS_CLEAR(TWCR,TWINT));
    /* Read Data */
    return TWDR;
}

uint8 TWI_getStatus(void)
{
    uint8 status;
    /* masking to eliminate first 3 bits and get the last 5 bits (status bits) */
    status = TWSR & 0xF8;
    return status;
}
