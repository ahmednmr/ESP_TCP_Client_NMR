/*
 * main.c
 *
 *  Created on: 5 Oct 2015
 *      Author: EmbeddedFab
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "MCAL/UART.h"
#include <avr/interrupt.h>
#include "uart.h"

#define TRUE 1
#define FALSE 0
#define DEFAULT_BUFFER_SIZE 100
char Rec_Data[DEFAULT_BUFFER_SIZE];
char Counter=0;




char Check_Respond(char * Expected_Respond);
char Check_Word_in_Respond(char * Word);
void Clear_REC_Buffer(void);

ISR (USART_RXC_vect)
{
	uint8_t oldsrg = SREG;
	cli();
	Rec_Data[Counter] = UDR;
	Counter++;
	if(Counter == DEFAULT_BUFFER_SIZE){
		Counter = 0; //pointer = 0;
	}
	SREG = oldsrg;
}


int main()
{
	char i=0,x=128,y=0,z=0,a=0,b=0,c=0,d=0;

	DDRD=(1<<PD4);
	init_UART();
	sei();


	UART_SEND_string("ATE0\r\n");
	while(!Check_Respond("\r\nOK\r\n"))
	{
		_delay_ms(1);
	}


	UART_SEND_string("AT+CWMODE=3\r\n");
	while(!Check_Respond("\r\nOK\r\n"))
	{
		_delay_ms(1);
	}

	UART_SEND_string("AT+CIPMUX=0\r\n");
	while(!Check_Respond("\r\nOK\r\n"))
	{
		_delay_ms(1);
	}
	UART_SEND_string("AT+CIPMODE=0\r\n");
	while(!Check_Respond("\r\nOK\r\n"))
	{
		_delay_ms(1);
	}
	UART_SEND_string("AT+CWJAP_DEF=\"Embeddedfab\",\"Embeddedfab135\"\r\n");

	while(!Check_Respond("WIFI DISCONNECT\r\nWIFI CONNECTED\r\nWIFI GOT IP\r\n\r\nOK\r\n"))
	{
		_delay_ms(1);
	}

	UART_SEND_string("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n");   //AT+CIPSTART="TCP","api.thingspeak.com",80
	while(!Check_Respond("CONNECT\r\n\r\nOK\r\n"))
	{
		_delay_ms(1);
	}

	UART_SEND_string("AT+CIPSEND=48\r\n");
	while(!Check_Respond("\r\nOK\r\n> "))
	{
		_delay_ms(1);
	}

	UART_SEND_string("GET /update?api_key=SD5OBD49N5H4O8RY&field2=80\r\n");
	while(!Check_Word_in_Respond("+IPD"))
	{
		_delay_ms(1);
	}

	_delay_ms(1000);
	Clear_REC_Buffer();

	UART_SEND_string("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n");   //AT+CIPSTART="TCP","api.thingspeak.com",80
	while(!Check_Respond("CONNECT\r\n\r\nOK\r\n"))
	{
		_delay_ms(1);
	}


	UART_SEND_string("AT+CIPSEND=37\r\n");
	while(!Check_Respond("\r\nOK\r\n> "))
	{
		_delay_ms(1);
	}

	UART_SEND_string("GET /channels/931047/feeds/last.txt\r\n");
	while(!Check_Word_in_Respond("+IPD"))
	{
		_delay_ms(1);
	}


	return 0;
}



char Check_Respond(char * Expected_Respond)
{
	char Respond_Length=0;

	Respond_Length=strlen(Expected_Respond);

	if(strncmp(Rec_Data,Expected_Respond, Respond_Length)==0)
	{
		Clear_REC_Buffer();
		return TRUE;
	}
	return FALSE;


}


char Check_Word_in_Respond(char * Word)
{


	if(strstr(Rec_Data,Word) != 0)
	{
		Clear_REC_Buffer();
		return TRUE;
	}

	return FALSE;
}

void Clear_REC_Buffer(void)
{
	Counter=0;
	memset(Rec_Data,0,DEFAULT_BUFFER_SIZE);
}
