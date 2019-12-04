/*
 * Nrf24.c
 *
 *  Created on: Dec 4, 2019
 *      Author: embeddedfab
 */

#include "Nrf24.h"
#include "../MCAL/spi.h"




	void init_Nrf24(void)
	{
	SPI_MasterInit();//EF_SpiInit(1);
	DDRD |=(1<<PD3);  //PD3 to CE
	DDRB |=(1<<PB4);  // PB4 to CSN

	PORTD &=~(1<<PD3);     //CE
	nrf24_csn_digitalWrite(HIGH);

	}



void nrf24_configRegister(char reg,char value)
{
	nrf24_csn_digitalWrite(LOW);

	SPI_Transmit(W_REGISTER | (REGISTER_MASK & reg));
	SPI_Transmit(value);
	nrf24_csn_digitalWrite(HIGH);
}

void nrf24_csn_digitalWrite(char state)
{
	if(state)
	{
		PORTB |=(1<<PB4);
	}
	else
	{
		PORTB &=~(1<<PB4);
	}
}


void nrf24_config(char channel, char pay_length)
{

	payload_len = pay_length;

	// Set RF channel
	nrf24_configRegister(RF_CH,channel);

	// Set length of incoming payload
	nrf24_configRegister(RX_PW_P0, 0x00); // Auto-ACK pipe ...
	nrf24_configRegister(RX_PW_P1, payload_len); // Data payload pipe
	nrf24_configRegister(RX_PW_P2, 0x00); // Pipe not used
	nrf24_configRegister(RX_PW_P3, 0x00); // Pipe not used
	nrf24_configRegister(RX_PW_P4, 0x00); // Pipe not used
	nrf24_configRegister(RX_PW_P5, 0x00); // Pipe not used


	// 1 Mbps, TX gain: 0dbm
	nrf24_configRegister(RF_SETUP, (0<<RF_DR)|((0x03)<<RF_PWR));

	// CRC enable, 1 byte CRC length
	nrf24_configRegister(CONFIG,nrf24_CONFIG);

	// Auto Acknowledgment
	nrf24_configRegister(EN_AA,(1<<ENAA_P0)|(1<<ENAA_P1)|(0<<ENAA_P2)|(0<<ENAA_P3)|(0<<ENAA_P4)|(0<<ENAA_P5));

	// Enable RX addresses
	nrf24_configRegister(EN_RXADDR,(1<<ERX_P0)|(1<<ERX_P1)|(0<<ERX_P2)|(0<<ERX_P3)|(0<<ERX_P4)|(0<<ERX_P5));

	// Auto retransmit delay: 1000 us and Up to 15 retransmit trials
	nrf24_configRegister(SETUP_RETR,(0x04<<ARD)|(0x0F<<ARC));

	// Dynamic length configurations: No dynamic length
	nrf24_configRegister(DYNPD,(0<<DPL_P0)|(0<<DPL_P1)|(0<<DPL_P2)|(0<<DPL_P3)|(0<<DPL_P4)|(0<<DPL_P5));


	nrf24_powerUpRx();

}



void nrf24_powerUpRx()
{
	nrf24_csn_digitalWrite(LOW);
	SPI_Transmit(FLUSH_RX);
	nrf24_csn_digitalWrite(HIGH);

	nrf24_configRegister(STATUS,(1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT));

	nrf24_ce_digitalWrite(LOW);
	nrf24_configRegister(CONFIG,nrf24_CONFIG|((1<<PWR_UP)|(1<<PRIM_RX)));
	nrf24_ce_digitalWrite(HIGH);
}


void nrf24_ce_digitalWrite(char state)
{
	if(state)
	{
		PORTD |=(1<<PD3);
	}
	else
	{
		PORTD &=~(1<<PD3);
	}
}


void nrf24_tx_address(char* adr)
{
	/* RX_ADDR_P0 must be set to the sending addr for auto ack to work. */
	nrf24_writeRegister(RX_ADDR_P0,adr,nrf24_ADDR_LEN);
	nrf24_writeRegister(TX_ADDR,adr,nrf24_ADDR_LEN);
}



/* Write to a single register of nrf24 */
void nrf24_writeRegister(char reg, char* value, char len)
{
	nrf24_csn_digitalWrite(LOW);
	SPI_Transmit(W_REGISTER | (REGISTER_MASK & reg));
	nrf24_transmitSync(value,len);
	nrf24_csn_digitalWrite(HIGH);
}

/* send multiple bytes over SPI */
void nrf24_transmitSync(char* dataout,char len)
{
	uint8_t i;

	for(i=0;i<len;i++)
	{
		SPI_Transmit(dataout[i]);
	}

}

void nrf24_transferSync(char* dataout,char* datain,char len)
{
	uint8_t i;

	for(i=0;i<len;i++)
	{
		datain[i] = SPI_Transmit(dataout[i]);
	}

}


void nrf24_rx_address(char * adr)
{
	nrf24_ce_digitalWrite(LOW);
	nrf24_writeRegister(RX_ADDR_P1,adr,nrf24_ADDR_LEN);
	nrf24_ce_digitalWrite(HIGH);
}


// Sends a data package to the default address. Be sure to send the correct
// amount of bytes as configured as payload on the receiver.
void nrf24_send(char* value)
{
	/* Go to Standby-I first */
	nrf24_ce_digitalWrite(LOW);

	/* Set to transmitter mode , Power up if needed */
	nrf24_powerUpTx();

	/* Do we really need to flush TX fifo each time ? */
#if 1
	/* Pull down chip select */
	nrf24_csn_digitalWrite(LOW);

	/* Write cmd to flush transmit FIFO */
	SPI_Transmit(FLUSH_TX);

	/* Pull up chip select */
	nrf24_csn_digitalWrite(HIGH);
#endif

	/* Pull down chip select */
	nrf24_csn_digitalWrite(LOW);

	/* Write cmd to write payload */
	SPI_Transmit(W_TX_PAYLOAD);

	/* Write payload */
	nrf24_transmitSync(value,payload_len);

	/* Pull up chip select */
	nrf24_csn_digitalWrite(HIGH);

	/* Start the transmission */
	nrf24_ce_digitalWrite(HIGH);
}


void nrf24_powerUpTx()
{
	nrf24_configRegister(STATUS,(1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT));

	nrf24_configRegister(CONFIG,nrf24_CONFIG|((1<<PWR_UP)|(0<<PRIM_RX)));
}


char nrf24_isSending()
{
	char status;

	/* read the current status */
	status = nrf24_getStatus();

	/* if sending successful (TX_DS) or max retries exceded (MAX_RT). */
	if((status & ((1 << TX_DS)  | (1 << MAX_RT))))
	{
		return 0; /* false */
	}

	return 1; /* true */

}




char nrf24_getStatus()
{
	char rv;
	nrf24_csn_digitalWrite(LOW);
	rv = SPI_Transmit(NOP);
	nrf24_csn_digitalWrite(HIGH);
	return rv;
}


char nrf24_lastMessageStatus()
{
	char rv;

	rv = nrf24_getStatus();

	/* Transmission went OK */
	if((rv & ((1 << TX_DS))))
	{
		return NRF24_TRANSMISSON_OK;
	}
	/* Maximum retransmission count is reached */
	/* Last message probably went missing ... */
	else if((rv & ((1 << MAX_RT))))
	{
		return NRF24_MESSAGE_LOST;
	}
	/* Probably still sending ... */
	else
	{
		return 0xFF;
	}
}


/* Returns the number of retransmissions occured for the last message */
char nrf24_retransmissionCount()
{
	char rv;
	nrf24_readRegister(OBSERVE_TX,&rv,1);
	rv = rv & 0x0F;
	return rv;
}

/* Read single register from nrf24 */
void nrf24_readRegister(char reg, char* value, char len)
{
    nrf24_csn_digitalWrite(LOW);
    SPI_Transmit(R_REGISTER | (REGISTER_MASK & reg));
    nrf24_transferSync(value,value,len);
    nrf24_csn_digitalWrite(HIGH);
}


void nrf24_powerDown()
{
    nrf24_ce_digitalWrite(LOW);
    nrf24_configRegister(CONFIG,nrf24_CONFIG);
}

/* Checks if data is available for reading */
/* Returns 1 if data is ready ... */
char nrf24_dataReady()
{
    // See note in getData() function - just checking RX_DR isn't good enough
	char status = nrf24_getStatus();

    // We can short circuit on RX_DR, but if it's not set, we still need
    // to check the FIFO for any pending packets
    if ( status & (1 << RX_DR) )
    {
        return 1;
    }

    return !nrf24_rxFifoEmpty();;
}


/* Reads payload bytes into data array */
void nrf24_getData(char* data)
{
    /* Pull down chip select */
    nrf24_csn_digitalWrite(LOW);

    /* Send cmd to read rx payload */
    SPI_Transmit( R_RX_PAYLOAD );

    /* Read payload */
    nrf24_transferSync(data,data,payload_len);

    /* Pull up chip select */
    nrf24_csn_digitalWrite(HIGH);

    /* Reset status register */
    nrf24_configRegister(STATUS,(1<<RX_DR));
}


char nrf24_rxFifoEmpty()
{
	char fifoStatus;

    nrf24_readRegister(FIFO_STATUS,&fifoStatus,1);

    return (fifoStatus & (1 << RX_EMPTY));
}
