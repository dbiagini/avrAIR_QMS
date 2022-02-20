/*
 * main.c
 *
 * Created: 12/1/2021 2:07:51 PM
 *  Author: dabi
 */ 

#ifndef F_CPU
#define F_CPU 16000000L
#endif

#include "./usart.h"
#include <stdlib.h>
#include <util/delay.h>
#include "./pms.h"
#include "./timer.h"
#include "./dht11.h"
#include "./ssd1331.h"
#include "string.h"


#define ERROR_MAX 10

#define BAUD 9600                           // define baud

#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)    // set baudrate value for UBRR

#define DEBUG_UART_OUT
//#define ECHO_MAIN
//#define UART_TEST
//#define PMS_DATA
//#define DHT_TEST
//#define TIMER_TEST
#define REAL_MAIN


//#ifdef (ECHO_MAIN || UART_TEST || PMS_DATA || DHT_TEST || TIMER_TEST)
#if defined (ECHO_MAIN)  || defined (UART_TEST) || defined (PMS_DATA) || defined (DHT_TEST) || defined (TIMER_TEST)
//#ifdef ECHO_MAIN
#include "ot_tests.h"
#endif


#define PM25_LOW 35
#define PM25_HIGH 75

#define PM1_LOW 35
#define PM1_HIGH 75

#define PM10_LOW 50
#define PM10_HIGH 110

//Globals
static char value[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
static int pms[3] = {0,0,0};
static int dht_data[5] = { 0, 0, 0, 0, 0};


// function to initialize UART
void uart_init (void)
{
	UBRRH=(BAUDRATE>>8);
	UBRRL=BAUDRATE;                         //set baud rate
	UCSRB = (1<<TXEN)|(1<<RXEN)|(1 << RXCIE)|(1 << UDRIE);             //enable receiver and transmitter, Enable receive complete interrupt
	
	UCSRC |= (1<<URSEL)|(3<<UCSZ0);// 8bit data format
}

void drawPage(int page){

	//time_t now;
	//struct tm *timenow;
	//draw time//
	//time(&now);
	//timenow = localtime(&now);
	
	//char hour[3] = {(char) value[timenow->tm_hour / 10], (char) value[(timenow->tm_hour % 10)], '\0'};
	//char minutes[3] = {(char) value[timenow->tm_min / 10], (char) value[(timenow->tm_min % 10)], '\0'};
	
	//#ifdef READ_TEMP
	////temp and humidity
	char temperature[3] = {(char) value[dht_data[2]/10],(char) value[dht_data[2] % 10], '\0' };
	char humidity[3] = {(char) value[dht_data[0]/10],(char) value[dht_data[0] % 10], '\0' };
	//#endif

	//SSD1331_string(0, 0, hour, 12,1, BLUE);
	//SSD1331_string(12, 0, ":", 12, 1, RED);
	//SSD1331_string(16, 0, minutes,12, 1, BLUE);

	
	//color coding values//
	int colors[3] = {GREEN, GREEN, GREEN};
	if ((pms[0] >= PM1_LOW) &&  (pms[0] < PM1_HIGH)) colors[0] = YELLOW;
	else if(pms[0] >= PM1_HIGH) colors[0] = RED;
	if ((pms[1] >= PM25_LOW) &&  (pms[1] < PM25_HIGH)) colors[1] = YELLOW;
	else if(pms[1] >= PM25_HIGH) colors[1] = RED;
	if ((pms[2] >= PM10_LOW) &&  (pms[2] < PM10_HIGH)) colors[2] = YELLOW;
	else if(pms[2] >= PM10_HIGH) colors[2] = RED;

	switch(page){
		
		case 0:		
		
		SSD1331_string(32, 0, temperature, 12,1, YELLOW);
		SSD1331_string(44, 0, "C", 12, 1, YELLOW);
		SSD1331_string(56, 0, humidity, 12,1, WHITE);
		SSD1331_string(68, 0, "%", 12, 1, WHITE);

		SSD1331_char1616(0, 13, value[pms[0] / 100], colors[0]);
		SSD1331_char1616(16, 13, value[(pms[0] % 100) / 10], colors[0]);
		SSD1331_char1616(32, 13, value[pms[0] % 10 ], colors[0]);
		SSD1331_string(50, 13, "PM 1.0", 12, 1, WHITE);

		SSD1331_char1616(0, 31, value[pms[1] / 100], colors[1]);
		SSD1331_char1616(16, 31, value[(pms[1] % 100) / 10], colors[1]);
		SSD1331_char1616(32, 31, value[pms[1] % 10 ], colors[1]);
		SSD1331_string(50, 31, "PM 2.5", 12, 1, WHITE);
		
		SSD1331_char1616(0, 49, value[pms[2] / 100], colors[2]);
		SSD1331_char1616(16, 49, value[(pms[2] % 100) / 10], colors[2]);
		SSD1331_char1616(32, 49, value[pms[2] % 10 ], colors[2]);
		SSD1331_string(50, 49, "PM 10", 12, 1, WHITE);
		

		break;
		case 1:
		//draw pm 2.5//
		SSD1331_char3216(0, 15, value[pms[1] / 100], colors[1]);
		SSD1331_char3216(24, 15, value[(pms[1] % 100) / 10], colors[1]);
		SSD1331_char3216(48, 15, value[pms[1] % 10 ], colors[1]);
		SSD1331_string(18, 49, "ug/m3 PM 2.5", 12, 1, WHITE);

		break;
		case 2:
		//draw pm 10.0//
		SSD1331_char3216(0, 15, value[pms[2] / 100], colors[2]);
		SSD1331_char3216(24, 15, value[(pms[2] % 100) / 10], colors[2]);
		SSD1331_char3216(48, 15, value[pms[2] % 10 ], colors[2]);
		SSD1331_string(18, 49, "ug/m3 PM 10", 12, 1, WHITE);
		break;
		//#ifdef READ_TEMP
		case 3:
		//print temperature and humidity
		SSD1331_char1616(0, 18, value[dht_data[0] / 10], WHITE);
		SSD1331_char1616(16, 18, value[dht_data[0] % 10 ], WHITE);
		SSD1331_string(32, 18, " %", 16, 1, WHITE);

		SSD1331_char1616(0, 40, value[dht_data[2] / 10], WHITE);
		SSD1331_char1616(16, 40, value[dht_data[2] % 10 ], WHITE);
		SSD1331_string(32, 40, ".", 16, 1, WHITE);
		SSD1331_char1616(40, 40, value[dht_data[3] / 10 ], WHITE);
		SSD1331_char1616(56, 40, value[dht_data[3] % 10 ], WHITE);
		SSD1331_string(72, 40, " C", 16, 1, WHITE);


		//SSD1331_char3216(18, 15, value[dht_data[2] / 10], colors[1]);
		//SSD1331_char3216(42, 15, value[dht_data[1] % 10 ], colors[1]);
		//SSD1331_string(18, 49, "ug/m3 PM 2.5", 12, 1, WHITE);
		
		//#endif
		default:
		break;
	}

}

void print_pmsToUsart (PMS5003_DATA in_data) {
	// usart is expected to be initialized before calling this function
	char out_value[10];
	char pm1_0_string[]="\n pm1.0: ";
	char pm2_5_string[]="\n pm2.5: ";
	char pm2_10_string[]="\n pm2.10: ";
	itoa(in_data.pm1cf, out_value, 10);
	usart_send(pm1_0_string, 9);
	usart_send(out_value, 2);
	itoa(in_data.pm2_5cf, out_value, 10);
	usart_send(pm2_5_string, 9);
	usart_send(out_value, 2);
	itoa(in_data.pm10cf, out_value, 10);
	usart_send(pm2_10_string, 10);
	usart_send(out_value, 2);
	
}

int main (void)
{
	
cli();

//Setup UART
usart_init();                            // initialize UART
sei();
	   
#ifdef ECHO_MAIN
test_pms_echo();
#endif

#ifdef UART_TEST
test_uart();
#endif

#ifdef PMS_DATA
test_pms_parseData();
#endif

#ifdef TIMER_TEST
test_timer();
#endif

#ifdef DHT_TEST
test_dht();
#endif


#ifdef REAL_MAIN

#ifdef DEBUG_UART_OUT
char message[]= "AIR QMS avr \n";
usart_send(message, 13);
#endif

//setup the pms sensor//
pms_init();
#ifdef DEBUG_UART_OUT
char buffer[] = "PMS initialized \n";
usart_send(buffer, 17);
#endif

int err_cnt = 0;
int valid_pms_Data = 0;
int valid_dht_Data = 0;

PMS5003_DATA d;
uint8_t dht_temp[5] = {0, 0, 0, 0, 0};

//start the OLED driver and clear the screen
SSD1331_begin();
SSD1331_clear();

_delay_ms(1000);//wait for the sensors to stabilize

while(1)
{
	//get sensor status//
	uint8_t status = read_pms5003_data(&d);
	if (status != UART_OK) {
		#ifdef DEBUG_UART_OUT
		output_uart_code(status);
		#endif
		
		//err_cnt++;
		//if (err_cnt > ERROR_MAX) {
			//char error_pms[] = "Too Many errors \n";
			//usart_send(error_pms, 17);
		//}
			//printf("airQMS: uart %d errors, stopping!. \n", err_cnt);
			//Run forever for now
			//break;
			//} else {
				//for (int i = 0; i < sizeof(pms); i++){
					//pms[i] = 0;
				//}
				valid_pms_Data = 0;
			//}
		} else {

		err_cnt =0; //reset max retries
		pms[0]= d.pm1cf;
		pms[1]= d.pm2_5cf;
		pms[2]= d.pm10cf;
		valid_pms_Data = 1;
		#ifdef DEBUG_UART_OUT
		print_pmsToUsart(d);
		#endif
	}
	
	if(DHT11ReadData(dht_temp)){
		
		//printf("cannot read DHT data");
		char err_msg[] = "cannot read DHT data \n";
		usart_send(err_msg, 22);
		valid_dht_Data = 0;

		}else{
		
		//memcpy(dht_data, dht_temp, sizeof(dht_data));
		dht_data[0] = dht_temp[0];
		dht_data[1] = dht_temp[1];
		dht_data[2] = dht_temp[2];
		dht_data[3] = dht_temp[3];
		valid_dht_Data = 1;
	}
	if(valid_pms_Data || valid_dht_Data) drawPage(0);		
				
//500 ms is needed to space enough the readings from dht sensor
	_delay_ms(500);
}

SSD1331_clear();

#endif

	return 0;
}
