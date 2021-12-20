/******************************************************************************

 This is a port of library https://github.com/vogelrh/pms5003c from Robert Vogel 
 The original library is meant to work on a raspbian/linux OS, here I've integrated
 it with an AVR UART library.
 C library for retrieving data from the Plantower PMS5003 particulate sensor.
 Modeled after the Pimoroni / pms5003-python library. 
 This library only provides UART communication with the sensor. It 
 does not use the hardware SET_CONTROL or RESET_CONTROL pins of the PMS5003.
 
 Copyright (C) 2019 Robert Vogel
 

********************************************************************************/

#ifndef F_CPU
#define F_CPU 8000000L
#endif

#define BAUD 9600                           // define baud for the uart

#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)    // set baudrate value for UBRR

#include "./usart.h"
#include <stdlib.h>
#include <stdint.h>
#include <util/delay.h>
#include "./timer.h"

/**
 * Library UART error codes
 */
#define UART_OK 0
#define UART_NO_DATA 1
#define UART_NOT_INITIALIZED 2
#define UART_INIT_ERROR 10
#define UART_PARAMETER_ERROR 11
#define UART_TX_ERROR 20
#define UART_RX_ERROR 21
#define UART_TIMEOUT_ERROR 22
#define UART_UNEXPECTED_DATA_ERROR 30
#define UART_CHECKSUM_ERROR 31

/**
 * Structure containing the pms5003 sensor data.
 */

typedef struct pms5003_data
{
	uint16_t pm1cf;   // PM1.0 ug/m3 (ultrafine particles)
	uint16_t pm2_5cf; // PM2.5 ug/m3 (combustion particles, organic compounds, metals)
	uint16_t pm10cf;  // PM10 ug/m3  (dust, pollen, mould spores)
	uint16_t pm1at;   // PM1.0 ug/m3 (atmos env)
	uint16_t pm2_5at; // PM2.5 ug/m3 (atmos env)
	uint16_t pm10at;  // PM10 ug/m3 (atmos env)
	uint16_t gt0_3;   // >0.3um in 0.1L air
	uint16_t gt0_5;   // >0.5um in 0.1L air
	uint16_t gt1;     // >1.0um in 0.1L air
	uint16_t gt2_5;   // >2.5um in 0.1L air
	uint16_t gt5;     // >5.0um in 0.1L air
	uint16_t gt10;    // >10um in 0.1L air
} PMS5003_DATA;

/**
* Initializes and configures the UART for the PMS5003 using the devices
* default parameters (optimized for the Raspberry Pi).
* The UART is set up for device Serial0, 9600 baud, 8-bit, no parity,
* no check bit, one stop bit, read-write in a nonblocking mode.
*
*/

void pms_init();

/**
 * Reads the pms5003 and populates the specified pms5003_data structure.
 * 
 * param[out]       data        data stucture to populate.
 * 
 * Returns a UART status/error code.
 */

int read_pms5003_data(PMS5003_DATA *data);

/**
 * Outputs a specific error/ status message to STDERR
 * 
 * param[in]        error_code      The UART function status/error code.
 */
void output_uart_code(int error_code);
