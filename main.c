/*
 * main.c
 *
 *  Created on: 5 Oct 2015
 *      Author: EmbeddedFab
 */

#include <avr/io.h>
#include <avr/delay.h>
#include <stdio.h>
#include <string.h>
#include "HAL/Nrf24.h"
#include "MCAL/UART.h"

char temp;
char q = 0;
char data_array[4];
char tx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
char rx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};



int main()
{


	init_UART();
	UART_SEND_string("System init\r\n");
	//.......init nrf24.......................................

	init_Nrf24();

	UART_SEND_string("SPI init Done\r\n");
	//.......write config  nrf24..............................

	/* Channel #2 , payload length: 4 */
	nrf24_config(2,4);
	UART_SEND_string("nrf24 Config done \r\n");
	//.....................................................

	nrf24_tx_address(tx_address);
	nrf24_rx_address(rx_address);

	UART_SEND_string("Start Prog\r\n");

	while(1)
	{


        if(nrf24_dataReady())
        {
            nrf24_getData(data_array);
            UART_SEND_string("Data recieved ----->>");

            UART_SendChar(data_array[0]);

            UART_SendChar(data_array[1]);
            UART_SendChar(data_array[2]);
            UART_SendChar(data_array[3]);
            UART_SEND_string("\r\n\r\n");
        }


	}
	return 0;
}


