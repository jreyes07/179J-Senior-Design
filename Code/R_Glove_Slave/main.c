
/*
* ATmega16 Interface with MPU-6050
* http://www.electronicwings.com
*
*/


#define F_CPU 8000000UL									/* Define CPU clock Frequency e.g. here its 8MHz */
#include <avr/io.h>										/* Include AVR std. library file */
#include <util/delay.h>									/* Include delay header file */
#include <inttypes.h>									/* Include integer type header file */
#include <stdlib.h>										/* Include standard library file */
#include <stdio.h>										/* Include standard library file */
#include <avr/interrupt.h>
#include "MPU6050_res_define.h"							/* Include MPU6050 register define file */
#include "I2C_Master_H_file.h"							/* Include I2C Master header file */
//#include "nokia5110.h"
#include "scheduler.h"
// #include "keypad.h"
#include "bit.h"
#include "usart_ATmega1284.h"

//#include "USART_RS232_H_file.h"							/* Include USART header file */

volatile float Acc_x,Acc_y,Acc_z,Temperature,Gyro_x,Gyro_y,Gyro_z;
volatile static float Xa,Ya,Za,t;
volatile static unsigned char ON = 0x00;//0x00 means off > 0x00 means start reading information and sending workout movement data
static unsigned char choice = 0x00;
void MPU6050_Init()										/* Gyro initialization function */
{
	_delay_ms(150);										/* Power up time >100ms */
	I2C_Start_Wait(0xD0);								/* Start with device write address */
	I2C_Write(SMPLRT_DIV);								/* Write to sample rate register */
	I2C_Write(0x07);									/* 1KHz sample rate */
	I2C_Stop();

	I2C_Start_Wait(0xD0);
	I2C_Write(PWR_MGMT_1);								/* Write to power management register */
	I2C_Write(0x01);									/* X axis gyroscope reference frequency */
	I2C_Stop();

	I2C_Start_Wait(0xD0);
	I2C_Write(CONFIG);									/* Write to Configuration register */
	I2C_Write(0x00);									/* Fs = 8KHz */
	I2C_Stop();

	I2C_Start_Wait(0xD0);
	I2C_Write(GYRO_CONFIG);								/* Write to Gyro configuration register */
	I2C_Write(0x18);									/* Full scale range +/- 2000 degree/C */
	I2C_Stop();

	I2C_Start_Wait(0xD0);
	I2C_Write(INT_ENABLE);								/* Write to interrupt enable register */
	I2C_Write(0x01);
	I2C_Stop();
}

void MPU_Start_Loc()
{
	I2C_Start_Wait(0xD0);								/* I2C start with device write address */
	I2C_Write(ACCEL_XOUT_H);							/* Write start location address from where to read */
	I2C_Repeated_Start(0xD1);							/* I2C start with device read address */
}

void Read_RawValue()
{
	MPU_Start_Loc();									/* Read Gyro values */
	Acc_x = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Acc_y = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Acc_z = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Temperature = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Gyro_x = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Gyro_y = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Gyro_z = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Nack());
	I2C_Stop();
}

enum SM1_States { init,wait,gyro};
int SM1_Tick(int state)
{//Start of SM1_Tick
	//======Local Variables=========

	static unsigned char buffer[20];
	static unsigned char float_[10];
	
	//=====TRANSITIONS==========
	switch(state){
		case init:
		state = wait;
		break;
		
		case wait:
		if(ON > 0x00)
		{
			state = gyro;
		}
		
		break;
		
		case gyro:
		// 		if(ON == 0x00)
		// 		{
		// 			state = wait;
		// 		}
		break;
		
		default:
		state = init;
		break;
	}//end of transitions
	
	//========ACTIONS===========
	switch(state){
		case init:
		
		break;
		
		case wait:
		Xa = 0;
		Ya = 0;
		Za = 0;
		break;
		
		case gyro:

		Read_RawValue();
		Xa = Acc_x/16384.0;								/* Divide raw value by sensitivity scale factor to get real values */
		Ya = Acc_y/16384.0;
		Za = Acc_z/16384.0;
		break;
		
		default:
		state = init;
		break;
	}//end of actions
	return state;
}//end of SM1_Tick

enum SM2_States{init2,wait2,send_SB, send_JJ,send_PU,send_SU};//send info for "ShadowBoxing", "JumpingJacks", "PushUps", "SitUps"
int SM2_Tick(int state)
{//start of SM2_Tick
	static double count;
	static  float gyro_accel_x;
	static  float gyro_accel_y;
	static  float gyro_accel_z;
	//=====TRANSITIONS==========
	switch(state){
		case init2:

		state = wait2;
		break;
		
		case wait2:
		//breakdown: SB = 1, SU = 2, JJ = 3, PU = 4

		// 		choice = ON;
		if(ON == '1')
		{
			// 			PORTA = 0x00;
			state = send_SB;
		}
		else if(ON == '2')
		{
			state = send_JJ;
		}
		else if(ON == '3')
		{
			state = send_PU;
		}
		else if(ON == '4')
		{
			state = send_SU;
		}
		break;
		
		case send_SB://0x01
		// 		if(ON == 0x00)
		// 		{
		// 			state = wait2;
		// 		}
		break;
		
		case send_JJ://0x02
		if(ON == 0x00)
		{
			state = wait2;
		}
		break;
		
		case send_PU://0x03
		if(ON == 0x00)
		{
			state = wait2;
		}
		break;
		
		case send_SU://0x04
		if(ON == 0x00)
		{
			state = wait2;
		}
		break;
		
		default:
		state = init2;
		break;
	}//end of transitions
	
	//========ACTIONS===========
	switch(state){
		case init2:
		count = 0;
		gyro_accel_x = 0;
		gyro_accel_y = 0;
		gyro_accel_z = 0;
		break;
		
		case wait2://wait to receive signal from Central hub that workout is about to begin
		if(USART_HasReceived(1))
		{
			ON = USART_Receive(1);
			PORTA = ON;
			choice = ON;
			USART_Flush(1);
		}
		// 		PORTA = 0x00;
		break;
		
		case send_SB:

		// 		if(USART_HasReceived(1))
		// 		{
		// 			ON = USART_Receive(1);
		// 			PORTA = ON;
		// 			USART_Flush(1);
		// 		}
		// 		count++;
		if(ON > 0x00)
		{
			count++;
			if( (Ya >= 1.4) && (gyro_accel_y < 1.4) && (count >= 20))
			{
				count = 0;
				unsigned char sending = 0x01;
				USART_Flush(1);
				if(USART_IsSendReady(1))
				USART_Send(sending,1);//send a bit to progress state of exercise
				while(!USART_HasTransmitted(1))
				{
					//wait until transmittedP
					PORTA = 0x00;
				}
				gyro_accel_y = Ya;
				PORTA = 0x01;
			}
			else if((Ya < 1.4) && (gyro_accel_y >= 1.4) && (count >=20))
			{
				count = 0;
				unsigned char sending = 0x00;
				USART_Flush(1);
				if(USART_IsSendReady(1))
				USART_Send(sending,1);//send a bit to progress state of exercise
				while(!USART_HasTransmitted(1))
				{
					//wait until transmittedP
					PORTA = 0x00;
				}
				gyro_accel_y = Ya;
				PORTA = 0x01;
			}
		}
		// 		else if (count >=20)
		// 		{
		// 			if(Ya >= 2)
		// 			{
		// 				unsigned char sending = 0x01;
		// 				USART_Flush(0);
		// 				if(USART_IsSendReady(0))
		// 				USART_Send(sending,0);//send a bit to progress state of exercise
		// 				while(!USART_HasTransmitted(0))
		// 				{
		// 					//wait until transmittedP
		// 					PORTA = 0x00;
		// 				}
		//
		// 				PORTA = 0x01;
		// 			}
		// 		}
		// 		else if (count >=30)
		// 		{
		// 			if(Ya >= 1 && Za >= 1)
		// 			{
		// 				unsigned char sending = 0x01;
		// 				USART_Flush(0);
		// 				if(USART_IsSendReady(0))
		// 				USART_Send(sending,0);//send a bit to progress state of exercise
		// 				while(!USART_HasTransmitted(0))
		// 				{
		// 					//wait until transmittedP
		// 					PORTA = 0x00;
		// 				}
		//
		// 				PORTA = 0x01;
		// 			}
		// 		}

		// 		else if(Za < 0.5)//resting fist is going to be pointed up so Za won't much applied force
		// 		{
		// 			USART_Flush(0);
		// 			if(USART_IsSendReady(0))
		// 			USART_Send(0x00,0);//send a bit to progress state of exercise
		// 			while(!USART_HasTransmitted(0))
		// 			{
		// 				//wait until transmitted
		// 			}
		// 		}
		break;
		
		case send_JJ://jumping jacks receive data
		// 		if(USART_HasReceived(0))
		// 		{
		// 			ON = USART_Receive(0);
		// 			PORTA = ON;
		// 			USART_Flush(0);
		// 		}
		
		if(Xa >= 0.4 && Za >= 0.5)
		{
			USART_Flush(1);
			if(USART_IsSendReady(1))
			USART_Send(0x01,1);//send a bit to progress state of exercise
			while(!USART_HasTransmitted(1))
			{
				//wait until transmitted
			}
			
		}
		else if(Xa < 0.4)//hands will be moving down for jumping jacks
		{
			USART_Flush(1);
			if(USART_IsSendReady(1))
			USART_Send(0x00,1);//send a bit to progress state of exercise
			while(!USART_HasTransmitted(1))
			{
				//wait until transmitted
			}
		}
		break;
		
		case send_PU:
		// 		if(USART_HasReceived(0))
		// 		{
		// 			ON = USART_Receive(0);
		// 			PORTA = ON;
		// 			USART_Flush(0);
		// 		}
		
		if(Za >= 0.5)//send data once user is in push-up position
		{
			USART_Flush(1);
			if(USART_IsSendReady(1))
			USART_Send(0x01,1);//send a bit to progress state of exercise
			while(!USART_HasTransmitted(1))
			{
				//wait until transmitted
			}
		}
		break;
		
		case send_SU:
		// 		if(USART_HasReceived(0))
		// 		{
		// 			ON = USART_Receive(0);
		// 			PORTA = ON;
		// 			USART_Flush(0);
		// 		}
		if(Za < 0)//send data once user is in sit-up position
		{
			USART_Flush(1);
			if(USART_IsSendReady(1))
			USART_Send(0x01,1);//send a bit to progress state of exercise
			while(!USART_HasTransmitted(1))
			{
				//wait until transmitted
			}
		}
		else if(Xa > 0.5)
		{
			USART_Flush(1);
			if(USART_IsSendReady(1))
			USART_Send(0x00,1);//send a bit to progress state of exercise
			while(!USART_HasTransmitted(1))
			{
				//wait until transmitted
			}
		}
		break;
		
		
		default:
		state = init2 ;
		break;
	}//end of actions
	return state;
}//end of SM2_Tick



int main()
{
	DDRA = 0xFF;//output for LED to show workout is in session
	PORTA = 0x00;
	// 	unsigned char tmpA;//test/misc. variable
	
	I2C_Init();											/* Initialize I2C */
	MPU6050_Init();										/* Initialize MPU6050 */
	initUSART(1);	//0 for right hand					/* Initialize USART with 9600 baud rate */
	//initUSART(1); //1 for left hand
	
	//declare number of tasks using tasksNum
	tasksNum = 2; //Task 1 can be reading information from Gyroscope
	//Task 2 can be sending gyroscope information via usart
	//initialize the task array: task tsks[2];
	task tsks[2];
	//set the task array: tasks = tsks;
	tasks = tsks;
	
	//define tasks
	unsigned char i=0; // task counter
	//Task1
	tasks[i].state=-1;
	tasks[i].period = 50;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &SM1_Tick;
	++i;
	//Task2
	tasks[i].state=-1;
	tasks[i].period = 50;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &SM2_Tick;
	//until all statemachines have been written.

	
	
	
	TimerSet(50);
	TimerOn();
	
	
	
	while(1)
	{
		
	}
}

//put all these functions and readings into state machines
//=======ADDITIONAL CODE THAT COULD BE USED FOR DIFFERENT DATA OF GYROSCOPE============
// 		Xg = Gyro_x/16.4;
// 		Yg = Gyro_y/16.4;
// 		Zg = Gyro_z/16.4;

// 		t = (Temperature/340.00)+36.53;					/* Convert temperature in °/c using formula */
//
//
//
// 		dtostrf( Xa, 3, 2, float_ );//converts Xa value to float_
// 		/* Take values in buffer to send all parameters over USART */
// 		sprintf(buffer," Ax = %s g\t",float_);
// 		// 		USART_SendString(buffer);
//
// 		nokia_lcd_set_cursor(0, 0);
// 		nokia_lcd_write_string(buffer,1);
// 		nokia_lcd_render();
//
//
//
// 		dtostrf( Ya, 3, 2, float_ );
// 		sprintf(buffer," Ay = %s g\t",float_);
// 		// 		USART_SendString(buffer);
// 		nokia_lcd_set_cursor(0, 10);
// 		nokia_lcd_write_string(buffer,1);
// 		nokia_lcd_render();

//
// 		dtostrf( Za, 3, 2, float_ );
// 		sprintf(buffer," Az = %s g\t",float_);
// 		// 		USART_SendString(buffer);
// 		nokia_lcd_set_cursor(0, 20);
// 		nokia_lcd_write_string(buffer,1);
// 		nokia_lcd_render();
//
// 		dtostrf( t, 3, 2, float_ );
// 		sprintf(buffer," T = %s%cC\t",float_,0xF8);           /* 0xF8 Ascii value of degree '°' on serial */
// 		USART_SendString(buffer);
//
// 		dtostrf( Xg, 3, 2, float_ );
// 		sprintf(buffer," Gx = %s%c/s\t",float_,0xF8);
// 		USART_SendString(buffer);
//
// 		dtostrf( Yg, 3, 2, float_ );
// 		sprintf(buffer," Gy = %s%c/s\t",float_,0xF8);
// 		USART_SendString(buffer);
//
// 		dtostrf( Zg, 3, 2, float_ );
// 		sprintf(buffer," Gz = %s%c/s\r\n",float_,0xF8);
// 		USART_SendString(buffer);
