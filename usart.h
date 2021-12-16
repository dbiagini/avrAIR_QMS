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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define USART_MOD 1

extern const char comma PROGMEM;
extern const char newline[3] PROGMEM;

void usart_init(void);
void usart_send(char * buffer, uint8_t len);
void usart_byte(uint8_t data);
void usart_send_pmem(PGM_P data, uint8_t len);
uint8_t usart_recv(char * data, uint8_t len, uint16_t o_timeout);
