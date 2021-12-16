/******************************************************************************

	I have found and modified this AVR IRQ based simple driver here https://usedbytes.com/software/usart_lib/
	the modifications I've done are mainly aimed at handling the overflow of the buffers so that we lose data but don't get stuck.
	After modifying this I've found https://github.com/andygock/avr-uart which seems a much more solid option than the one below

	// Serial driver for AVR USART
	// Uses 24 bytes for buffers etc
	// This work is provided as-is without any warranty whatsoever
	// Use it at your own risk
	// Modify and redistribute at will, as long as this disclaimer remains
	// (C) Brian Starkey, 2011

********************************************************************************/

#ifndef F_CPU
#define F_CPU 8000000L
#endif
#ifndef BAUD
#define BAUD 9600                           // define baud for the uart
#endif
#ifndef BAUDRATE
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)    // set baudrate value for UBRR
#endif
#include <util/delay.h>
#include "usart.h"
#define RX_BUF_SIZE 16  //works only for 2^x sizes
#define TX_BUF_SIZE 4  //works only for 2^x sizes
#define MAX_8BITS 0xFF

const char comma PROGMEM = ',';
const char newline[] PROGMEM = "\r\n";

volatile uint8_t tx_buffer[TX_BUF_SIZE];
volatile uint8_t rx_buffer[RX_BUF_SIZE];
volatile uint8_t tx_bytes = 0;
volatile uint8_t rx_bytes = 0;
volatile uint8_t tx_buffer_pos = 0;
volatile uint8_t rx_buffer_pos = 0;

void usart_init() {

	UBRRH=(BAUDRATE>>8);
	UBRRL=BAUDRATE;                         //set baud rate
	UCSRB = (1<<TXEN)|(1<<RXEN)|(1 << RXCIE)|(1 << UDRIE);             //enable receiver and transmitter, Enable receive complete interrupt
		
	UCSRC |= (1<<URSEL)|(3<<UCSZ0);// 8bit data format
}

void usart_send(char * data, uint8_t len) {
	uint8_t i;
	for (i = 0; i < len; i++) {
		while (tx_bytes > (tx_buffer_pos + (TX_BUF_SIZE-1)));
		uint8_t index = tx_bytes & (TX_BUF_SIZE-1);
		tx_buffer[index] = data[i];
		if(tx_bytes == MAX_8BITS) {
			//reset indexes when overflowing
			tx_bytes = (tx_bytes & (TX_BUF_SIZE-1)) + TX_BUF_SIZE;
			tx_buffer_pos = tx_buffer_pos & (RX_BUF_SIZE-1);
		}
		tx_bytes++;
		UCSRB |= (1 << UDRIE);
	}	
}

void usart_send_pmem(PGM_P data, uint8_t len) {
	char in_ram;
	uint8_t i;
	for (i = 0; i < len; i++) {
		in_ram = pgm_read_byte(&data[i]);
		usart_send(&in_ram, 1);
	};
}

uint8_t usart_recv(char * data, uint8_t len, uint16_t o_timeout) {
	uint8_t i;
	uint16_t timeout = o_timeout;
	for (i = 0; i < len; i++) {
		while (rx_bytes >= rx_buffer_pos) {
			_delay_us(250);
			if ((timeout-- == 0) && (o_timeout > 0)) {
				return i;
			}
		}
		timeout = o_timeout;
		if (rx_bytes < rx_buffer_pos) {
			data[i] = rx_buffer[rx_bytes & (RX_BUF_SIZE-1)];
			rx_bytes++;
		}
	}
	return i;
}

ISR(USART_UDRE_vect) {
	if (tx_buffer_pos < tx_bytes) {
		UDR = tx_buffer[(tx_buffer_pos++ & (TX_BUF_SIZE-1))];
	}
	else {
		//not sure about this case
		UCSRB &= ~(1 << UDRIE);
		tx_bytes = 0;
		tx_buffer_pos = 0;
	}
}

ISR(USART_RXC_vect) {	
	//handle overflow//
	if(rx_buffer_pos == MAX_8BITS) {
		
		rx_buffer_pos = (rx_buffer_pos & (RX_BUF_SIZE-1)) + RX_BUF_SIZE;
		rx_bytes = rx_bytes & (RX_BUF_SIZE-1);		
	}
	rx_buffer[rx_buffer_pos++ & (RX_BUF_SIZE-1)] = UDR;
	//additions better handling of slow reads, dropping bytes//
	if(rx_buffer_pos >= (rx_bytes + RX_BUF_SIZE)) rx_bytes = rx_buffer_pos - (RX_BUF_SIZE - 1);
}    
