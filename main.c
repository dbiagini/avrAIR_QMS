/*
 * main.c
 *
 * Created: 12/1/2021 2:07:51 PM
 *  Author: dabi
 */ 

#ifndef F_CPU
#define F_CPU 16000000L
#endif

//#define TIMER_H
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

//#define ECHO_MAIN
//#define UART_TEST
//#define PMS_DATA
//#define DHT_TEST
//#define OLED_TEST
//#define TIMER_TEST
//#ifndef F_CPU
//#define F_CPU 16000000UL                    // set the CPU clock
//#endif
//#include <util/delay.h>

//#define TIMER_TEST

#define REAL_MAIN

#define PM25_LOW 35
#define PM25_HIGH 75

#define PM1_LOW 35
#define PM1_HIGH 75

#define PM10_LOW 50
#define PM10_HIGH 110

//Globals
char value[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
int pms[3] = {0,0,0};
int dht_data[5] = { 0, 0, 0, 0, 0};


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
		//print temp humidity//
		
		//#ifdef READ_TEMP
		////temp and humidity
		SSD1331_string(32, 0, temperature, 12,1, YELLOW);
		SSD1331_string(44, 0, "C", 12, 1, YELLOW);
		SSD1331_string(56, 0, humidity, 12,1, WHITE);
		SSD1331_string(68, 0, "%", 12, 1, WHITE);
		//#endif

		SSD1331_char1616(0, 13, value[pms[0] / 100], colors[0]);
		SSD1331_char1616(16, 13, value[(pms[0] % 100) / 10], colors[0]);
		SSD1331_char1616(32, 13, value[pms[0] % 10 ], colors[0]);
		SSD1331_string(50, 13, "PM 1.0", 12, 1, WHITE);
		//printf("pm 1.0 %d \n", pms[0]);

		SSD1331_char1616(0, 31, value[pms[1] / 100], colors[1]);
		SSD1331_char1616(16, 31, value[(pms[1] % 100) / 10], colors[1]);
		SSD1331_char1616(32, 31, value[pms[1] % 10 ], colors[1]);
		SSD1331_string(50, 31, "PM 2.5", 12, 1, WHITE);
		
		SSD1331_char1616(0, 49, value[pms[2] / 100], colors[2]);
		SSD1331_char1616(16, 49, value[(pms[2] % 100) / 10], colors[2]);
		SSD1331_char1616(32, 49, value[pms[2] % 10 ], colors[2]);
		SSD1331_string(50, 49, "PM 10", 12, 1, WHITE);
		
		//printf("pm 2.5 %d \n", pms[1]);
		//printf("pm 10 %d \n", pms[2]);

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

// main function: entry point of program
int main (void)
{
	
cli();
	   
#ifdef ECHO_MAIN

char a[2];
char buffer[10] = "Test ";
//char buffer1[10] = "erro ";
char buffer2[4];

char input_pms[32];

uart_init();                            // initialize UART
sei(); 
usart_send(buffer, 5);
while(1)
{

    int ret = 0;
	int bytes = 1; 
	a[0]=0;
	a[1]=0x65;
	ret = usart_recv(&a[0], bytes, 1);                   // save the received data in a variable
	//itoa(a,buffer,10);                  // convert numerals into string
	if((ret == bytes)&&(a[0]==0x42)){
		//uart_transmit( a );
		//usart_send(&a[0], 1);
		//uint8_t value[2] = 0x424d;
			ret = usart_recv(&a[1], bytes, 5);                   // look for second part
			if((ret == bytes)&&(a[1]==0x4d)){
				input_pms[0]= a[0];
				input_pms[1]= a[1];
				//PORTD |= 0x80;
				for(int i=0; i<=28; i++){
					ret = usart_recv(&input_pms[i+2], bytes, 5);                   // save the received data in a variable 
					if(ret != bytes){
						itoa(ret, buffer2, 10);
						usart_send(buffer2, 4);
						//PORTD |= 0x80;
						break;
					}
				}	
				usart_send(input_pms, 30);
			}else{
				//PORTD |= 0x80;
				itoa(ret, buffer2, 10);
				usart_send(buffer2, 4);
				//usart_send((char*)&ret, 1);
			}
			//_delay_ms(400);
	}else{
		//PORTD |= 0x80;
		//itoa(ret, buffer2, 10);
		//usart_send(buffer2, 4);
		                     // wait before next attempt
	}
	//_delay_ms(200);
}
#endif
#ifdef UART_TEST

char buffer[10] = "Test ";
	
uart_init();                            // initialize UART
sei(); 
while(1)
{
	//a=uart_recieve();                   // save the received data in a variable
	//itoa(a,buffer,10);                  // convert numerals into string
	for (int i=0; i<5; i++){
		usart_send(buffer,9);
		_delay_ms(100);                     // wait before next attempt
	}
}
#endif
#ifdef PMS_DATA

char buffer[] = "PMS initialized \n";
pms_init();
sei();
usart_send(buffer,18);
PMS5003_DATA d;
char out_value[10];
//char gen_err[]="\n Error: ";
char pm1_0_string[]="\n pm1.0: ";
char pm2_5_string[]="\n pm2.5: ";
char pm2_10_string[]="\n pm2.10: ";

int status = UART_OK;
while(1)
{
	status = read_pms5003_data(&d);
	if (status != UART_OK) {
			output_uart_code(status);
		} else {
		itoa(d.pm1cf, out_value, 10);
		usart_send(pm1_0_string, 9);
		usart_send(out_value, 2);				
		itoa(d.pm2_5cf, out_value, 10);
		usart_send(pm2_5_string, 9);
		usart_send(out_value, 2);
		itoa(d.pm10cf, out_value, 10);
		usart_send(pm2_10_string, 10);
		usart_send(out_value, 2);
	}
	//_delay_ms(100);
}

#endif
#ifdef TIMER_TEST
	usart_init();                            // initialize UART
	// Set timer to use function timerFunc
	//setTimerCallback(timerFunc);
	// Start timer with an interval of 1.5 seconds
	char test_end[] = "Test End \n";
	char next_line[] = "\n";
	char success[10] = "Success \n";
	char failure[10] = "Failure \n";
	char test_start[10] = "Test";
	char seconds[2]; // up to 99
	char milliseconds[4]; // up to 9999
	sei();
	startTimer_s();
	usart_send(test_start,5); //sends two bites out
	usart_send(next_line,1); //sends two bites out
	uint16_t previous = 0;
	while(1)
	{
		uint16_t secs = getTimer();
		if (previous != secs){
			itoa(secs, seconds, 10);
			usart_send(seconds,2); //sends two bites out
			usart_send(next_line,1); //sends two bites out
			previous = secs;
		}
		if(previous >= 10) break;
		_delay_ms(500);
	}
	stopTimer();
	_delay_ms(2000);
	uint16_t secs = getTimer();
	if(secs == 0) usart_send(success, 10); //we're expecting 0
	else {
		usart_send(failure, 10);
		itoa(secs, seconds, 10);
		usart_send(seconds,2); 
	}
	//test ms
	usart_send(test_start,5); 
	usart_send(next_line,1); //sends two bites out
	previous = 0;
	startTimer_ms();
	while(1)
	{
		uint16_t mills = getTimer();
		//itoa(mills, milliseconds, 10);
		//usart_send(milliseconds,4); //sends two bites out
		//usart_send(next_line,1); //sends two bites out
		if (previous != mills){
			itoa(mills, milliseconds, 10);
			usart_send(milliseconds,4); //sends two bites out
			usart_send(next_line,1); //sends two bites out
			previous = mills;
		}
		if(previous >= 2000) break;
		_delay_ms(100);
	}
	usart_send(test_end,10); 
	stopTimer();

#endif

#ifdef DHT_TEST

	char message[]= "DHT11 test \n";
	char message1[]= "DHT11 Fail \n";
	char temperature[2]= "  ";
	char next_line[1]= "\n";
	uint8_t ret = DHT_OK;
	uint8_t data[5] = { 0, 0, 0, 0, 0 };
	usart_init();                            // initialize UART
	sei();

	usart_send(message, 12);


	while ( 1 )
	{
		_delay_ms( 3000 );
		if( DHT_OK == DHT11ReadData(data)){
			char temp[3];
			char temp_d[3];
			char hum[3];
			char hum_d[3];
			char next_line[1]= "\n";
			char dot[1]=".";
			itoa(data[0], hum, 10);
			itoa(data[1], hum_d, 10);
			itoa(data[2], temp, 10);
			itoa(data[3], temp_d, 10);
			usart_send(hum, 3);
			usart_send(dot, 1);
			usart_send(hum_d, 3);
			usart_send(next_line, 1);
			usart_send(temp, 3);
			usart_send(dot, 1);
			usart_send(temp_d, 3);
			usart_send(next_line, 1);
		} else {
			usart_send(message1, 12);
			//_delay_ms( 3000 );
			//break;
		}
		
		//if( DHT_OK == read_dht11_dat(data)){
			////itoa(data[2], temperature, 10);
			////usart_send(temperature, 2);
			////usart_send(next_line, 1);
			//} else {
			//usart_send(message1, 12);
			////_delay_ms( 3000 );
			////break;
		//}
	}


#endif

#ifdef OLED_TEST


char message[]= "OLED test \n";
char message1[]= "bitmap write \n";
char next_line[1]= "\n";

usart_init();                            // initialize UART
sei();
usart_send(message, 12);


while ( 1 )
{
	_delay_ms( 3000 );
	SSD1331_begin();
	//_delay_ms( 2000 );
	SSD1331_clear();
	
	usart_send(message, 14);
	/* display an image of bitmap matrix */
	//SSD1331_mono_bitmap(0, 0, waveshare_logo, 96, 64, BLUE);
	_delay_ms( 2000 );
	
	SSD1331_clear();
}

#endif

#ifdef REAL_MAIN

//Setup UART
char message[]= "AIR QMS avr \n";

usart_init();                            // initialize UART
sei();
usart_send(message, 13);

//setup the sensor//
pms_init();

char buffer[] = "PMS initialized \n";
usart_send(buffer, 17);

int err_cnt = 0;
int valid_Data = 0;
PMS5003_DATA d;

SSD1331_begin();
SSD1331_clear();

_delay_ms(1000);//wait for the sensors to stabilize

while(1)
{
	
	//get sensor status//
	uint8_t status = read_pms5003_data(&d);
	if (status != UART_OK) {
		output_uart_code(status);
		err_cnt++;
		if (err_cnt > ERROR_MAX) {
			char error_pms[] = "Too Many errors \n";
			usart_send(error_pms, 17);
			//printf("airQMS: uart %d errors, stopping!. \n", err_cnt);
			break;
			} else {
				for (int i = 0; i < sizeof(pms); i++){
					pms[i] = 0;
				}
				valid_Data = 0;
			}
		} else {

		err_cnt =0; //reset max retries
		pms[0]= d.pm1at;
		pms[1]= d.pm2_5at;
		pms[2]= d.pm10at;
		valid_Data = 1;
	}
	
	if (valid_Data){
		//#ifdef READ_TEMP
		uint8_t dht_temp[5] = {0, 0, 0, 0, 0};
		if(DHT11ReadData(dht_temp)){
		
			//printf("cannot read DHT data");
			char err_msg[] = "cannot read DHT data \n";
			usart_send(buffer, 22);

			}else{
		
			//memcpy(dht_data, dht_temp, sizeof(dht_data));
			dht_data[0] = dht_temp[0];
			dht_data[1] = dht_temp[1];
			dht_data[2] = dht_temp[2];
			dht_data[3] = dht_temp[3];
			drawPage(0);
		}
		//#endif
		//SSD1331_string(0, 52, "MUSIC", 12, 0, WHITE);
		//SSD1331_string(64, 52, "MENU", 12, 1, WHITE);
		//printf("pm 2.5 %d \n", pms[1]);
		//printf("pm 10 %d \n", pms[2]);
	//drawPage(0);
	//SSD1331_display();
	//printf("%d\n", numberOfPresses);

	}
	_delay_ms(500);
}

SSD1331_clear();


#endif

	return 0;
}
