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


#include "pms.h"
#include <string.h> 


#define PMS5003_EXPECTED_BYTES 28
#define LITTLE_ENDED 1
static const uint16_t PMS5003_SOF = 19778; //the x42, x4d start characters

/**************************************************
 * Module level variables.
 **************************************************/

int uart_status = UART_NOT_INITIALIZED;

/**
 * Union for converting two byte reads from the UART to 
 * unsigned short int.
 * NOTE: This will ONLY work on Big Endian systems!!!
 * If using on Little Endian systems then the bytes must
 * be swapped before readin value;
 */
typedef union uart_word {
  uint16_t value;
  uint8_t data[2];
} uart_word;

/**
 * Union for converting the 26 bytes of data fread 
 * from the pms5003 to unsigned int values.
 * NOTE: Same Big Endian issues apply!
 */

typedef union pms5003_data_block {
	struct
	{
		uint16_t pm1cf;
		uint16_t pm2_5cf;
		uint16_t pm10cf;
		uint16_t pm1at;
		uint16_t pm2_5at;
		uint16_t pm10at;
		uint16_t gt0_3;
		uint16_t gt0_5;
		uint16_t gt1;
		uint16_t gt2_5;
		uint16_t gt5;
		uint16_t gt10;
		uint16_t reserved;
		uint16_t cksum;
	} d;
	uint8_t raw_data[PMS5003_EXPECTED_BYTES];
} pms5003_data_block;

/**************************************************
 * Private functions
 **************************************************/

/**
 * Swaps bytes in two byte words in byte array
 */
static void swap_bytes16(uint8_t *array, int size)
{
	uint8_t temp;
	int i;

	for (i = 0; i < size; i = i + 2)
	{
		temp = array[i];
		array[i] = array[i + 1];
		array[i + 1] = temp;
	}
}

/**
 * Reads two bytes from the UART and populates the specified uart_word
 * union. Note because we are running in no-wait mode, we may not have 
 * any data yet. In that case we make sure the word structure has a
 * zero value.
 * 
 * param[out]       word        A uart_word union recieving the data.
 * 
 * Returns a UART status/error code.
 */

static int read_word(uart_word *word, int swap)
{
	int rx_cnt = usart_recv(( char*) word->data, 2, 5);
	if (rx_cnt != 2) {
		word->value = 0;
		return UART_NO_DATA;
	}	
	if (swap)
	{
		swap_bytes16(word->data, 2);
	}
	
	return UART_OK;
}

/**
 * Read a block of bytes from the PMS5003 and populates the specified
 * pms5003_data_block union. If an error occurs, then an error code is
 * returned. If successful, UART_OK is returned.
 * 
 * param[out]       data        A pms5003_data_block union receiving the data.
 * 
 * Returns a UART status/error code.
 */
static int read_pms_data_block(pms5003_data_block *data)
{
  int i;
  int checksum = 0;
  int rstat = 0;

  // get time at start an loop through reading 2 bytes from uart until
  // we find a start sequence or we exceed the timeout interval.
  uint16_t elapsed_seconds;

  // Zero the data structure
  for (i = 0; i < PMS5003_EXPECTED_BYTES; i++)
  {
    data->raw_data[i] = 0;
  }
  // Exit if UART is not open
  if (uart_status != UART_OK)
  {
    return UART_NOT_INITIALIZED;
  }

  startTimer1_s(); //start counting seconds
  uart_word sof;

  while (1)  //using timer to check if 5 seconds have elapsed
  { //wait for SOF word
    elapsed_seconds = getTimer1(); //get seconds elapsed
    if (elapsed_seconds >= 5)
    {
	  stopTimer1();
      return UART_TIMEOUT_ERROR;
    }
    rstat = read_word(&sof, 0);
    if (rstat == UART_OK && sof.value == PMS5003_SOF)
    {
      checksum = sof.data[0] + sof.data[1];
      break;
    }
  }

  stopTimer1();

  // Now read frame length word
  uart_word packet_length;
  rstat = read_word(&packet_length, LITTLE_ENDED);
  if (rstat != UART_OK)
  {
	usart_send((char*) packet_length.data , 2);
    return rstat;
  }


  if (packet_length.value != PMS5003_EXPECTED_BYTES)
  {
    return UART_UNEXPECTED_DATA_ERROR;
  }

  checksum += (packet_length.data[0] + packet_length.data[1]);

  // Now read sensor data
  int rx_cnt = usart_recv( (char*) data->raw_data, PMS5003_EXPECTED_BYTES, 5);
  if (rx_cnt != PMS5003_EXPECTED_BYTES)
  {
    return UART_RX_ERROR; // some read error occured.
  }

  // Swap bytes if little ended system
  if (LITTLE_ENDED)
  {
    swap_bytes16(data->raw_data, PMS5003_EXPECTED_BYTES);
  }

  // Finish calculating the check sum
  for (i = 0; i < PMS5003_EXPECTED_BYTES - 2; i++)
  {
    checksum += data->raw_data[i];
  }
  if (checksum != data->d.cksum)
  {
    return UART_CHECKSUM_ERROR;
  }


  return UART_OK;
}

/**************************************************
* ***** Public Facing functions *****
**************************************************/
void output_uart_code(int error_code)
{
	char err_msg[72];
	
	switch (error_code)
	{
		case UART_OK:
		strcpy(err_msg,"Status: OK");
		break;
		case UART_NO_DATA:
		strcpy(err_msg,"Status: No data returned from UART");
		break;
		case UART_NOT_INITIALIZED:
		strcpy(err_msg,"Status: UART not initialized \n");
		break;
		case UART_INIT_ERROR:
		strcpy(err_msg,"ERROR: UART initialization error \n");
		break;
		case UART_PARAMETER_ERROR:
		strcpy(err_msg,"ERROR: Incorrect UART parameters provided \n");
		break;
		case UART_TX_ERROR:
		strcpy(err_msg,"ERROR: UART transmit problem \n");
		break;
		case UART_RX_ERROR:
		strcpy(err_msg,"ERROR: UART recive problem \n");
		break;
		case UART_UNEXPECTED_DATA_ERROR:
		strcpy(err_msg,"ERROR: PMS5003 unexpected data. Block size does not match data structure \n");
		break;
		case UART_CHECKSUM_ERROR:
		strcpy(err_msg,"ERROR: PMS5003 data checksum error \n");
		break;
		case UART_TIMEOUT_ERROR:
		strcpy(err_msg, "ERROR: PMS5003 read timeout error \n");
		break;
	}
	if (err_msg != NULL)
	{
			usart_send(err_msg,strlen(err_msg));
	}
}

void pms_init()
{
	usart_init();                            // initialize UART;
	uart_status = UART_OK;
}

int read_pms5003_data(PMS5003_DATA *data)
{
	pms5003_data_block rd;
	int status = read_pms_data_block(&rd); //read the sensor

	if (status == UART_OK)
	{ //transfer data to final structure
		data->pm1cf = rd.d.pm1cf;
		data->pm2_5cf = rd.d.pm2_5cf;
		data->pm10cf = rd.d.pm10cf;
		data->pm1at = rd.d.pm1at;
		data->pm2_5at = rd.d.pm2_5at;
		data->pm10at = rd.d.pm10at;
		data->gt0_3 = rd.d.gt0_3;
		data->gt0_5 = rd.d.gt0_5;
		data->gt1 = rd.d.gt1;
		data->gt2_5 = rd.d.gt2_5;
		data->gt5 = rd.d.gt5;
		data->gt10 = rd.d.gt10;
	}
	return status;
}