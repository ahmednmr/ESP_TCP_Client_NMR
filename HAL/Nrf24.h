/*
 * Nrf24.h
 *
 *  Created on: Dec 4, 2019
 *      Author: embeddedfab
 */

#ifndef NRF24_H_
#define NRF24_H_

#include "nrf24_cnf.h"

#define LOW 0
#define HIGH 1

#define nrf24_ADDR_LEN 5
#define nrf24_CONFIG ((1<<EN_CRC)|(0<<CRCO))

#define NRF24_TRANSMISSON_OK 0
#define NRF24_MESSAGE_LOST   1

char payload_len;

void init_Nrf24(void);
void nrf24_csn_digitalWrite(char state);
void nrf24_configRegister(char reg,char value);
void nrf24_config(char channel, char pay_length);
void nrf24_powerUpRx();
void nrf24_ce_digitalWrite(char state);
void nrf24_tx_address(char* adr);
void nrf24_writeRegister(char reg, char* value, char len);
void nrf24_transferSync(char* dataout,char* datain,char len);
void nrf24_transmitSync(char* dataout,char len);
void nrf24_rx_address(char * adr);
void nrf24_powerUpTx();
void nrf24_send(char* value);
char nrf24_isSending();
char nrf24_getStatus();
char nrf24_lastMessageStatus();
char nrf24_retransmissionCount();
void nrf24_readRegister(char reg, char* value, char len);
void nrf24_powerDown();
char nrf24_dataReady() ;
void nrf24_getData(char* data);
char nrf24_rxFifoEmpty();



#endif /* NRF24_H_ */
