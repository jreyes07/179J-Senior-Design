/*
 * CS122A Lab 2.c
 *
 * Created: 10/10/2017 11:33:57 AM
 * Author : User
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart_ATmega1284.h"

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;    // Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG = 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

enum States{Init, SendD1, SendD0,Follow} state;
void SyncLightsLead(){
	
	unsigned char tmpA; //Light. 1=on. 0=off
	switch(state){//transitions
		case Init:
		state = SendD1;
		break;
		
		case SendD1:
		state = SendD0;
		break;
		
		case SendD0:
		state = SendD1;
		break;
		
		default:
		break;
	}//transitions
	
	switch(state){//actions
	case Init:
	tmpA = 0x00;
	PORTA = tmpA;
	break;
	
	case SendD1:
	//send data
	tmpA=0x01;
	PORTA = tmpA;
	USART_Flush(0);
	if(USART_IsSendReady(0))
	USART_Send(tmpA,0);	
	while(!USART_HasTransmitted(0)){
		//wait until transmitted
	}
	break;
	
	case SendD0:
	tmpA=0x00;
	PORTA = tmpA;
	USART_Flush(0);
	if(USART_IsSendReady(0))
	USART_Send(tmpA,0);
	while(!USART_HasTransmitted(0)){
		//wait until transmitted
	}
	break;

	default:
	break;
	}//actions
	}
	
	enum States1{Init1, receive} stateR;
	void SyncLightsFollow(){
	switch(stateR){//transitions
	case Init1:
	stateR = receive;
	break;
	
	case receive:
	break;
	
	default:
	break;
	}//actions
	
	switch(stateR){//actions
	case Init1:
	break;
	
	case receive:
				if(USART_HasReceived(0))
				{
					//count=0;
					PORTA = USART_Receive(0);
					USART_Flush(0);
				}

	
	break;
	
	default:
	break;
	}
	}
	
int main(void)
{
    /* Replace with your application code */
	DDRA = 0xFF;//output
	PORTA = 0x00;//initialize
	DDRB = 0x00;//input
	PORTB = 0xFF;//initialize
	DDRC = 0xFF;//output
	PORTC = 0x00;//initialize
	
		TimerSet(1000);
		TimerOn();
		initUSART(0);//initializes USART0
		initUSART(1);//initializes USART1
		state = Init;
		stateR = Init1;
		unsigned char role = 0x00; //0x00 = Leader //0x01 = Follower
		
		
    while (1) 
    {//maybe make 2 functions. one for flllow and one for lead
		role = 0x01;

		if(role == 0x00)
		SyncLightsLead();
		else
		{
			TimerSet(50);
		SyncLightsFollow();
		}
		while(!TimerFlag);
		TimerFlag = 0;
    }
}

