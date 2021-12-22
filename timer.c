/******************************************************************************

	I have found and modified this AVR IRQ based simple driver here https://brokenbrain.se/avr/timer in order to count seconds 
	or milliseconds depending on which api is used to initialize the timer.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
	THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
	DEALINGS IN THE SOFTWARE.	

********************************************************************************/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "timer.h"
#define ONE_MILLISECOND 1000UL
#define PRESCALER_MS 8
#define PRESCALER_S 256
#define TICKS_PMS ((F_CPU/PRESCALER_MS)/ONE_MILLISECOND)
#define TICKS_PS (F_CPU/PRESCALER_MS)


#ifndef F_CPU
#define F_CPU 16000000L
#endif


//void (*timerCallback)();

volatile uint16_t total_units;  //currently the size of this variable is the limit for counting time units
uint16_t targetTime;

void startTimer_s() {
	// start timer with interrupt every second.
    total_units = 0 ; //initialize unit total
	// Enable timer, CLKio/8, Clear Timer on Compare
	TCCR1A |= 0;
	TCCR1B = (1 << WGM12)| (1 << CS12); // Mode = CTC, Prescaler = 256
	uint16_t ticks = (F_CPU/PRESCALER_S); //ticks for a second


	// Set timer to 0 as starting value
	TCNT1 = 0;

	// Disable force output compare
	//TCCR1C = 0;

	// Set timer compare to calculated value
	OCR1A = ticks;

	// Enable timer compare interrupt
	TIMSK |= (1 << OCIE1A);
	
}

void startTimer_ms() {
	// start timer with interrupt every millisecond.

    total_units = 0 ; //initialize unit total
	// Enable timer, CLKio/8, Clear Timer on Compare
	TCCR1A |= 0;
	TCCR1B = (1 << WGM12)| (1 << CS11); // Mode = CTC, Prescaler = 8
	uint16_t ticks = (F_CPU/PRESCALER_MS)/ONE_MILLISECOND;


	// Set timer to 0 as starting value
	TCNT1 = 0;

	// Disable force output compare
	//TCCR1C = 0;

	// Set timer compare to calculated value
	OCR1A = ticks;

	// Enable timer compare interrupt
	TIMSK |= (1 << OCIE1A);
}

void setTimer_ms(uint16_t in_time) {
	targetTime = in_time;
	startTimer_ms();
}
void setTimer_s(uint16_t in_time) {
	targetTime = in_time;
	startTimer_s();
}
uint16_t getTimer(){	
	
	uint16_t unit_to_ret = 0;
	
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		unit_to_ret = total_units;
	}
	return unit_to_ret ;
}
void stopTimer() {
	// disable timer interrupts
	TIMSK &= ~(1 << OCIE1A);
	total_units = 0; //reset units counter
}
//void setTimerCallback(void (*func)()) {
	//timerCallback = func;
//}

ISR(TIMER1_COMPA_vect) {
	//timer is cleared on compare
	total_units++;  //increment the counted time units
	//if (total_units == targetTime) {
		//total_units = 0;
		//timerCallback();
	//}
	
}
