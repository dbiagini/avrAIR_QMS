/*
 * dht11.h
 *
 * Created: 22/12/2021 11:33:30
 *  Author: dabi
 */ 



#ifndef DHT11_H
#define DHT11_H

#ifndef F_CPU
#define F_CPU 16000000L
#endif

#include <avr/io.h>

#define DHT_PORT	PORTC
#define MAXTIMINGS	85
#define DHT_PIN_BIT	PC7 //7
#define DHT_DDR		DDRC
#define DHT_PIN	    PINC //
#define DHT_OK      0
#define DHT_ERR     1

//uint8_t read_dht11_dat(uint8_t *dht_11_dat);
uint8_t DHT11ReadData(uint8_t *dht11_data);

#endif /* DHT11_H */