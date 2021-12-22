/*
 * dht11.c
 *
 * Created: 22/12/2021 11:34:46
 *  Author: dabi
 */ 
#ifndef F_CPU
#define F_CPU 16000000L
#endif

#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "./dht11.h"
#include "./usart.h"

#define HIGH 1
 
uint8_t read_dht11_dat(uint8_t *dht11_dat)
{
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j= 0, i;
	//float f;
	int ret = DHT_OK;
 
	//int dht11_dat[5] = { 0, 0, 0, 0, 0 };
	dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;
	
	/* Send START signal to sensor */
	DHT_DDR |= (1 << DHT_PIN_BIT); // set pin to output
	DHT_PORT &= ~(1 << DHT_PIN_BIT); // set pin LOW
	_delay_ms(20); // keep pin LOW for at least 18 ms
	
	/* Set Pin to High and set DDR to input to read data from sensor.  */
	DHT_PORT |= 1 << DHT_PIN_BIT; // set pin high
	DHT_DDR &= ~(1 << DHT_PIN_BIT); //set pin to input
	_delay_us(32); // wait for 20-40 us
	
 
	for ( i = 0; i < MAXTIMINGS; i++ )
	{
		counter = 0;
		while ((DHT_PORT & (1 << DHT_PIN_BIT)) == laststate )
		{
			counter++;
			_delay_us(1);
			if ( counter == 255 )
			{
				//PORTD |= 0x10;
				break;
			}
		}
		laststate = (DHT_PORT & (1 << DHT_PIN_BIT));
        
		if ( counter == 255 ){
			//printf("no response retries = %d \n", counter);
			break;
		} else {
		
			//printf("response after %d retries \n ", counter);
			//PORTD |= 0x10;
			/*char retries[3];
			char next_line[1]= "\n";
			itoa(counter, retries, 10);
			usart_send(retries, 3);
			usart_send(next_line, 1);*/
		
		}
 		//skip first 3 retries //
		if ( (i >= 4) && (i % 2 == 0) )
		{
			dht11_dat[j / 8] <<= 1;
			if ( counter > 16 )
				dht11_dat[j / 8] |= 1;
			j++;
		}
	
	}
 
	if ( (j >= 40) &&
	     (dht11_dat[4] == ( (dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF) ) )
	{
		//f = dht11_dat[2] * 9. / 5. + 32;
		/*printf( "Humidity = %d.%d %% Temperature = %d.%d C (%.1f F)\n",
			dht11_dat[0], dht11_dat[1], dht11_dat[2], dht11_dat[3], f );*/
					PORTD |= 0x10;

	} else {
		//printf( "Data not good, skip\n" );
		//ret = DHT_ERR;
		char temp[3];
		char temp_d[3];
		char hum[3];
		char hum_d[3];
		char next_line[1]= "\n";
		char dot[1]=".";
		itoa(dht11_dat[0], hum, 10);
		itoa(dht11_dat[1], hum_d, 10);
		itoa(dht11_dat[2], temp, 10);
		itoa(dht11_dat[3], temp_d, 10);
		usart_send(hum, 3);
		usart_send(dot, 1);
		usart_send(hum_d, 3);
		usart_send(next_line, 1);
		usart_send(temp, 3);
		usart_send(dot, 1);
		usart_send(temp_d, 3);
		usart_send(next_line, 1);
	}

	return ret;
}

uint8_t DHT11ReadData(uint8_t *dht11_data){
	uint8_t sensor_bytes, bits, buffer=0, timeout=0, checksum;
	
	/* Initialise sensor if flag is 0 then set to 1 to run only once */
	//if(DHT11Init == 0) DHT11Setup();
	
	/* Send START signal to sensor */
	DHT_DDR |= (1 << DHT_PIN_BIT); // set pin to output
	DHT_PORT &= ~(1 << DHT_PIN_BIT); // set pin LOW
	_delay_ms(20); // keep pin LOW for at least 18 ms

	/* Set DDR to input LOW (high Z) to read data from sensor. 
	The external pull-up resistor will pull the data line HIGH */
	DHT_DDR &= ~(1 << DHT_PIN_BIT);
	_delay_us(32); // wait for 20-40 us
	
	/* Listen for sensor response - 80us LOW and 80us HIGH signal */
	if(DHT_PIN & (1 << DHT_PIN_BIT)){ // If HIGH, sensor didn't respond
		return DHT_ERR; // error code
	}
	
	/* Sensor sent LOW signal, wait for HIGH */
	_delay_us(82);
	
	/* If HIGH, sensor is ready to send data */
	if(DHT_PIN & (1 << DHT_PIN_BIT)){
		_delay_us(82); // wait for HIGH signal to end
		if(DHT_PIN & (1 << DHT_PIN_BIT)) return DHT_ERR; // still HIGH - something is wrong
	}else{
		return DHT_ERR; // error code
	}
	
		//PORTD |= 0x10;

	/* Ready to read data from sensor */
	for(sensor_bytes=0; sensor_bytes<5; sensor_bytes++){
		/* Reset the buffer */
		buffer = 0;
			
		for(bits=0; bits<8; bits++){
			/* Wait 50 us between each bits while signal is LOW */
			while(~DHT_PIN & (1 << DHT_PIN_BIT)){
				/* Wait no more than 80 us. If the sensor breaks and remains LOW
				the MCU will not remain stuck in a while loop */
				timeout++;
				if(timeout > 8) break;
				_delay_us(10);
			}
			timeout = 0;
			
			/* Signal is HIGH - read the bit */
			if(DHT_PIN & (1 << DHT_PIN_BIT)){
				_delay_us(40); // 26-28 us HIGH means a 0 bit, 70 us means a 1 bit
				/* If signal is still HIGH means a 1 bit */
				if(DHT_PIN & (1 << DHT_PIN_BIT)){
					/* Put a 1 to buffer. Sensor sends MSB first */
					buffer |= 1 << (7-bits);
				}
				
				/* Wait for HIGH signal to end */
				while(DHT_PIN & (1 << DHT_PIN_BIT)){
					/* Wait no more than 80 us. If the sensor breaks and remains HIGH
					the MCU will not remain stuck in a while loop */
					timeout++;
					if(timeout > 8) break;
					_delay_us(10);
				}
				timeout = 0;
			}else{
				return 0; // signal still LOW. Return error response
			}
		}
		
		/* Dump the buffer to global array */
		dht11_data[sensor_bytes] = buffer;
	}
	
	/* Wait for data transmision to end. Sensor will output LOW for 50 us and then goes into
	low-power consumption mode until the next START command from the MCU */
	_delay_us(60);
	
	/* Set DDR to output */
	//DHT_DDR |= 1 << DHT_PIN_BIT;
	
	/* Set pin HIGH. When idle, sensor DATA line must be kept HIGH */
	//DHT_PORT |= 1 << DHT_PIN_BIT;
	
	/* Check for data transmission errors */
	checksum = dht11_data[0] + dht11_data[1] + dht11_data[2] + dht11_data[3];
	if(checksum != dht11_data[4]){
		return DHT_ERR; // checksum error code
	}
	
	//#ifdef ADD_MINIMUM_DELAY
		//_delay_ms(SAMPLE_DELAY);
	//#endif
	
	/* OK return code */
	return DHT_OK;
}