/*
 * main.c
 *
 * Created: 12/1/2021 2:07:51 PM
 *  Author: dabi
 */ 

#ifndef F_CPU
#define F_CPU 8000000L
#endif

//#define TIMER_H
#include "./usart.h"
#include <stdlib.h>
#include <util/delay.h>
#include "./pms.h"
#include "./timer.h"



#define BAUD 9600                           // define baud

#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)    // set baudrate value for UBRR

//#define ECHO_MAIN
//#define UART_TEST
//#define PMS_DATA
//#define TIMER_TEST
//#ifndef F_CPU
//#define F_CPU 16000000UL                    // set the CPU clock
//#endif
//#include <util/delay.h>

//#define TIMER_TEST



// function to initialize UART
void uart_init (void)
{
	UBRRH=(BAUDRATE>>8);
	UBRRL=BAUDRATE;                         //set baud rate
	UCSRB = (1<<TXEN)|(1<<RXEN)|(1 << RXCIE)|(1 << UDRIE);             //enable receiver and transmitter, Enable receive complete interrupt
	
	UCSRC |= (1<<URSEL)|(3<<UCSZ0);// 8bit data format
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
			//PORTD |= 0x80;
			//char err[4] = "    ";
			//itoa(status, err, 10);
			//usart_send(gen_err, 9);
			//usart_send(err, 2);
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
	_delay_ms(2000);
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

	return 0;
}
