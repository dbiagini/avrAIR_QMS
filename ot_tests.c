/*
 * ot_tests.c
 *
 * Created: 20/02/2022 12:37:55
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

#include "ot_tests.h"



void test_pms_echo(){
	
	//receive data from PMS sensor and output to UART as it is//

	char a[2];
	char buffer[10] = "Test ";
	char buffer2[4];
	char input_pms[32];

	usart_send(buffer, 5);

	while(1)
	{

		int ret = 0;
		int bytes = 1;
		a[0]=0;
		a[1]=0x65;
		ret = usart_recv(&a[0], bytes, 1);                   // save the received data in a variable
		if((ret == bytes)&&(a[0]==0x42)){
			ret = usart_recv(&a[1], bytes, 5);                   // look for second part
			if((ret == bytes)&&(a[1]==0x4d)){
				input_pms[0]= a[0];
				input_pms[1]= a[1];
				for(int i=0; i<=28; i++){
					ret = usart_recv(&input_pms[i+2], bytes, 5);                   // save the received data in a variable
					if(ret != bytes){
						itoa(ret, buffer2, 10);
						usart_send(buffer2, 4);
						break;
					}
				}
				usart_send(input_pms, 30);
				}else{
				itoa(ret, buffer2, 10);
				usart_send(buffer2, 4);
			}
		}
	}
	
	return;
	
}

void test_uart(){
	
	//test the uart functionality sending a Test string or echoing what is being received
	char buffer[10] = "Test ";
	while(1)
	{
		//a=uart_recieve();                   // save the received data in a variable
		//itoa(a,buffer,10);                  // convert numerals into string
		for (int i=0; i<5; i++){
			usart_send(buffer,9);
			_delay_ms(100);                     // wait before next attempt
		}
	}
	
	return;
	
}

void test_pms_parseData(){
	
	//receive data from PMS sensor parse it and output it to uart//
	char buffer[] = "PMS initialized \n";
	usart_send(buffer,18);
	PMS5003_DATA d;
	char out_value[10];
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
	}
	
	return;
	
}

void test_timer(){
	
	//testing the Timer functionality (polling) in seconds and milliseconds//
	// Set timer to use function timerFunc
	char test_end[] = "Test End \n";
	char next_line[] = "\n";
	char success[10] = "Success \n";
	char failure[10] = "Failure \n";
	char test_start[10] = "Test";
	char seconds[2]; // up to 99
	char milliseconds[4]; // up to 9999
	startTimer1_s();
	usart_send(test_start,5); //sends two bites out
	usart_send(next_line,1); //sends two bites out
	uint16_t previous = 0;
	while(1)
	{
		uint16_t secs = getTimer1();
		if (previous != secs){
			itoa(secs, seconds, 10);
			usart_send(seconds,2); //sends two bites out
			usart_send(next_line,1); //sends two bites out
			previous = secs;
		}
		if(previous >= 10) break;
		_delay_ms(500);
	}
	stopTimer1();
	_delay_ms(2000);
	uint16_t secs = getTimer1();
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
	startTimer1_ms();
	while(1)
	{
		uint16_t mills = getTimer1();
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
	stopTimer1();

	return;
	
}

void test_dht() {
	
	//testing the DHT seonsor output//

	char message[]= "DHT11 test \n";
	char message1[]= "DHT11 Fail \n";
	char next_line[1]= "\n";
	uint8_t data[5] = { 0, 0, 0, 0, 0 };

	usart_send(message, 12);


	while ( 1 )
	{
		_delay_ms( 3000 );
		if( DHT_OK == DHT11ReadData(data)){
			char temp[3];
			char temp_d[3];
			char hum[3];
			char hum_d[3];
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
		}
	}
	return;
	
}
