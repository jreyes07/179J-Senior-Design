/*
 * 5110 LCD.c
 *
 * Created: 11/20/2017 3:10:04 PM
 * Author : User
 */ 

//https://github.com/LittleBuster/avr-nokia5110
#include <avr/io.h>
#define F_CPU 8000000UL  // 1 MHz//should be the frequency the microcontroller runs at
#include <util/delay.h>
#include <stdio.h>
#include "nokia5110.h"
#include "scheduler.h"
#include "keypad.h"
#include "bit.h"


//    nokia_lcd_init();
//    nokia_lcd_clear();
//	  nokia_lcd_power();
//    nokia_lcd_write_string("Bella!",1);
//    nokia_lcd_set_cursor(0, 10);
//    nokia_lcd_write_string("De Cavia!", 1);
//    nokia_lcd_render();


enum SM1_States{Init,wait,boxingL,boxingR,boxingHL,boxingHR,boxingUL,boxingUR,finish1,finish2,weight};
int SM1_Tick(int state){
	//LOCAL VARIABLES
	//--------------------
	static int state_delay = 0;
	static unsigned char kpVal=0x00;
	static short rightweight=0;
	static unsigned short x;
	static unsigned char my_char;
	
	//TRANSITIONS
	switch(state){
		case Init:
		state_delay++;
		if(state_delay>=3)
		{
		state_delay=0;
		state = wait;
		}
		break;
		case wait:
		//switch to next state
		if(kpVal=='1')
		{
			state=boxingL;
		}
		break;
		case boxingL:
		state_delay++;
		state = boxingR;
		break;
		case boxingR:
		state_delay++;
		if(state_delay>=10)
		{
			state=boxingHL;
			state_delay=0;
		}
		else
		{
		state = boxingL;
		}
		break;
		case boxingHL:
		state_delay++;
		state=boxingHR;
		break;
		case boxingHR:
		state_delay++;
		if(state_delay>=10)
		{
			state=boxingUL;
			state_delay=0;
		}
		else
		{
			state=boxingHL;
		}
		break;
		case boxingUL:
		state_delay++;
		state=boxingUR;
		break;
		case boxingUR:
		state_delay++;
		if(state_delay>=10)
		{
			state=finish1;
			state_delay=0;
		}
		else
		{
			state=boxingUL;
		}
		break;
		case finish1:
		state_delay++;
		if(state_delay>=10)
		{
			state=finish2;
			state_delay=0;
		}
		break;
		case finish2:
		state=finish2;
		break;
		case weight:
		if(rightweight==10)
		{
			state=boxingL;
		}
		else
		state=weight;
		break;
		default:
		state=Init;
		break;
	}
	
	//ACTIONS
	switch(state){
		case Init:
		   nokia_lcd_init();
		   nokia_lcd_clear();
		   nokia_lcd_set_cursor(0, 0);
		   nokia_lcd_write_string("WELCOME",2);
		   nokia_lcd_render();

		   break;
		case wait:
		 nokia_lcd_clear();
		 nokia_lcd_set_cursor(0, 0);
		 nokia_lcd_write_string("Workout type:",1);
		 nokia_lcd_set_cursor(0,10);
		 nokia_lcd_write_string("1. Boxing",1);
		 nokia_lcd_set_cursor(0,20);
		 nokia_lcd_write_string("2. Sit-ups",1);
		 nokia_lcd_set_cursor(0,30);
		 nokia_lcd_write_string("3.Jumpin jacks",1);
		 nokia_lcd_set_cursor(0,40);
		 nokia_lcd_write_string("4.Run in place",1);
		 nokia_lcd_render();
		 kpVal= GetKeypadKey();
		break;
		case boxingL:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("LEFT", 2);
		nokia_lcd_set_cursor(0,30);
		nokia_lcd_write_string("JAB",2);
		nokia_lcd_render();
		break;
		case boxingR:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("RIGHT", 2);
		nokia_lcd_set_cursor(0,30);
		nokia_lcd_write_string("JAB",2);
		nokia_lcd_render();
		break;
		case boxingHL:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("LEFT", 2);
		nokia_lcd_set_cursor(0,30);
		nokia_lcd_write_string("HOOK",2);
		nokia_lcd_render();
		break;
		case boxingHR:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("RIGHT", 2);
		nokia_lcd_set_cursor(0,30);
		nokia_lcd_write_string("HOOK",2);
		nokia_lcd_render();
		break;
		case boxingUL:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("LEFT", 2);
		nokia_lcd_set_cursor(0,30);
		nokia_lcd_write_string("UPPERCUT",2);
		nokia_lcd_render();
		break;
		case boxingUR:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("RIGHT", 2);
		nokia_lcd_set_cursor(0,30);
		nokia_lcd_write_string("UPPERCUT",2);
		nokia_lcd_render();
		break;
		case finish1:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("GREAT", 2);
		nokia_lcd_set_cursor(0,25);
		nokia_lcd_write_string("WORKOUT",2);
		nokia_lcd_render();
		break;
		case finish2:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("BURNED CALORIES:", 1);
		nokia_lcd_set_cursor(0,15);
		nokia_lcd_write_string("15",3);
		nokia_lcd_render();
		break;
		case weight:
		
// 				x = ADC;
// 				my_char = (char)x;
// 				PORTD = my_char;
// //				my_char=(char)(x>>8);
		break;
		default:
		state=Init;
		break;
	}
	return state;
	};
	
void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}

int main(void)
{
	DDRD = 0xFF;//output to measure weight
	PORTD = 0x00;
	DDRC = 0xF0;//PORT for Keypad
	PORTC = 0xF0;//initialize port
	DDRB = 0xFF;//Nokia5110
	PORTB = 0x00;
	DDRA = 0x00;
	PORTA=0xFF;
	//ADC_init();
	
	tasksNum=1;
	task tsks[1];
	tasks=tsks;	
	unsigned char i=0;
	//Task1
	tasks[i].state=-1;
	tasks[i].period = 100;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &SM1_Tick;
	++i;

	TimerSet(1000);
	TimerOn();
	
	while(1)
	{

		
	}

}

