/******************************************************************************

	I have found and modified this AVR IRQ based simple driver here https://brokenbrain.se/avr/timer in order to count seconds 
	or milliseconds depending on which api is used to initialize the timer.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
	THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
	DEALINGS IN THE SOFTWARE.	

********************************************************************************/

#ifndef TIMER_H
#define TIMER_H

void setTimer1_ms(uint16_t milliseconds);
void setTimer1_s(uint16_t seconds);
void startTimer1_s();
void startTimer1_ms();
void stopTimer1();
uint16_t getTimer1(); //returns the time unit passed (either s or ms)
void setTimer1Callback(void (*func)());

#endif