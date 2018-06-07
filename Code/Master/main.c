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
#include "usart_ATmega1284.h"


//    nokia_lcd_init();
//    nokia_lcd_clear();
//	  nokia_lcd_power();
//    nokia_lcd_write_string("Bella!",1);
//    nokia_lcd_set_cursor(0, 10);
//    nokia_lcd_write_string("De Cavia!", 1);
//    nokia_lcd_render();


enum SM1_States{Init,
	age1,age1_press, age2,age2_press, age3,age3_press,age4,age4_press,//inputting age
	gender,gender_press,//acquire gender
	height_ft,height_ft_press,height_in1,height_in1_press,height_in2,height_in2_press, height_in3,height_in3_press,//acquire height in feet and inches
	bodyweight1,bodyweight1_press,bodyweight2,bodyweight2_press,bodyweight3,bodyweight3_press,bodyweight4,bodyweight4_press,//Acquire body weight in pounds
	dumb_bell_weight1,dumb_bell_weight1_press, dumb_bell_weight2,dumb_bell_weight2_press,dumb_bell_weight3,dumb_bell_weight3_press,//end of Personal Information
	ex_selection,ex_setup,ex_setup_isready,
	boxingL,boxingR,boxingHL,boxingHR,boxingUL,boxingUR,//Shadowboxing
	su_UP,su_DOWN,su_L_UP,su_L_DOWN,su_R_UP, su_R_DOWN,//Situps
	JJ_expand, JJ_close,//Jumping jacks exercise
	pushups_UP, pushups_DOWN,//Pushups
	finish1,finish2,
weight};
int SM1_Tick(int state){
	//LOCAL VARIABLES
	//--------------------
	static int state_delay = 0;//timing count for delay
	static unsigned char kpVal=0x00;//Keypad value
	static unsigned char age_1=0x00; //first digit of age (1's place)
	static unsigned char age_2=0x00; //second digit of age (10's place)
	static unsigned char age_3=0x00; //third digit of age (100's place)
	static unsigned char age_4=0x00;
	static unsigned short total_age = 0x00; //Numerical age value as a whole
	static enum gen{male,female} gen_var;
	// 	static enum gen gen_var;
	static unsigned char feet = 0x00;
	static unsigned char inches1 = 0x00;
	static unsigned char inches2 =0x00;
	static unsigned char inches3 = 0x00;
	static unsigned short inches_total = 0x00;//Numerical inches total as a whole
	static unsigned char bWeight1 = 0x00;//body weight 1st digit
	static unsigned char bWeight2 = 0x00;//body weight 2nd digit
	static unsigned char bWeight3 = 0x00;//body weight 3rd digit
	static unsigned char bWeight4 = 0x00;//extra variable for '#'
	static unsigned short bWeight_total = 0x00;//Numerical total body weight combining 3 different digits ( bWeight1, bWeight2, bWeight3)
	static unsigned char db_weight1 = 0x00;//first digit of weight in pounds
	static unsigned char db_weight2 = 0x00;//second digit of weight in pounds
	static unsigned char db_weight3 = 0x00;//extra variable for '#' button
	static unsigned short db_weight_total = 0x00;//Numerical total value of dumb-bell weights
	static float rightweight=0;//right weight is combination of bodyweight and dumbbell weight
	static unsigned short x = 0;
	static unsigned char my_char = 0x00;
	static unsigned char receive1 = 0x00;
	static unsigned char receive2 = 0x00;
	// 	static unsigned char receive3 = 0x00;
	// 	static unsigned char receive4 = 0x00;
	
	static float cals_burned;
	static unsigned char float_[10];

	
	//TRANSITIONS
	switch(state){
		case Init:
		state_delay++;
		if(state_delay>=3)
		{
			state_delay=0;
			state = age1;
		}
		break;
		
		case age1:
		if(kpVal=='#')
		{
			state = age4_press;
		}
		else if(kpVal>=48 && kpVal<=57)
		{
			state = age1_press;
		}
		break;
		
		case age1_press:
		if(kpVal== 0x00)
		{
			state=age2;
		}
		break;
		
		case age2:
		if(kpVal=='#')
		{
			state = age4_press;
		}
		else if(kpVal >=48 && kpVal <=57)
		{
			state = age2_press;
		}
		break;
		
		case age2_press:
		if(kpVal==0x00)
		{
			state = age3;
		}
		break;
		
		case age3:
		if(kpVal=='#')
		{
			state = age4_press;
		}
		else if(kpVal >=48 && kpVal <=57)
		{
			state = age3_press;
		}
		break;
		
		case age3_press:
		if(kpVal==0x00)
		{
			state = age4;
		}
		break;
		
		case age4:
		if(kpVal=='#')
		{
			state = age4_press;
		}
		break;
		
		
		case age4_press:
		if(kpVal == 0x00)
		{
			state = gender;
		}
		break;
		
		case gender:
		if(kpVal=='A' || kpVal=='B')
		{
			state=gender_press;
		}
		break;
		
		case gender_press:
		if(kpVal==0x00)
		{
			state=height_ft;
		}
		break;
		
		case height_ft:
		if(kpVal != 0x00)
		{
			state=height_ft_press;
		}
		break;
		
		case height_ft_press:
		if(kpVal==0x00)
		{
			state = height_in1;
		}
		break;
		
		case height_in1:
		if(kpVal >=48 && kpVal <=57)
		{
			state=height_in1_press;
		}
		if(kpVal == '#')
		{
			state = height_in3_press;
		}
		break;
		
		case height_in1_press:
		if(kpVal==0x00)
		{
			state=height_in2;
		}
		break;
		
		case height_in2:
		if(kpVal >=48 && kpVal <=57)
		{
			state=height_in2_press;
		}
		else if(kpVal=='#')
		{
			state = height_in3_press;
		}
		break;
		
		case height_in2_press:
		if(kpVal == 0x00)
		{
			state = height_in3;
		}
		break;
		
		case height_in3:
		if(kpVal == '#')
		{
			state = height_in3_press;
		}
		break;
		
		case height_in3_press:
		if(kpVal == 0x00)
		{
			state = bodyweight1;
		}
		break;
		
		case bodyweight1:
		if(kpVal=='#')
		{
			state=dumb_bell_weight1;
		}
		else if(kpVal >=48 && kpVal <=57)
		{
			state = bodyweight1_press;
		}
		break;
		
		case bodyweight1_press:
		if(kpVal == 0x00)
		{
			state = bodyweight2;
		}
		break;
		
		case bodyweight2:
		if(kpVal=='#')
		{
			state=bodyweight4_press;
		}
		else if(kpVal >=48 && kpVal <=57)
		{
			state = bodyweight2_press;
		}
		break;
		
		case bodyweight2_press:
		if(kpVal==0x00)
		{
			state = bodyweight3;
		}
		break;
		
		case bodyweight3:
		if(kpVal=='#')
		{
			state=bodyweight4_press;
		}
		else if(kpVal >=48 && kpVal <=57)
		{
			state = bodyweight3_press;
		}
		break;
		
		case bodyweight3_press:
		if(kpVal == 0x00)
		{
			state = bodyweight4;
		}
		break;
		
		case bodyweight4:
		if(kpVal=='#')
		{
			state=bodyweight4_press;
		}
		break;
		
		case bodyweight4_press:
		if(kpVal == 0x00)
		{
			state = dumb_bell_weight1;
		}
		break;
		
		case dumb_bell_weight1:
		if(kpVal=='#')
		{
			state=dumb_bell_weight3_press;
		}
		else if(kpVal >= 48 && kpVal <= 57)
		{
			state = dumb_bell_weight1_press;
		}
		break;
		
		case dumb_bell_weight1_press:
		if(kpVal == 0x00)
		{
			state = dumb_bell_weight2;
		}
		break;
		
		case dumb_bell_weight2:
		if(kpVal=='#')
		{
			state=dumb_bell_weight3_press;
		}
		else if(kpVal >= 48 && kpVal <= 57)
		{
			state = dumb_bell_weight2_press;
		}
		break;
		
		case dumb_bell_weight2_press:
		if(kpVal == 0x00)
		{
			state = dumb_bell_weight3;
		}
		break;
		
		case dumb_bell_weight3:
		if(kpVal=='#')
		{
			state=dumb_bell_weight3_press;
		}
		break;
		
		case dumb_bell_weight3_press:
		if(kpVal == 0x00)
		{
			state = ex_selection;
		}
		break;
		
		
		case ex_selection:
		//select which exercise to perform
		if(kpVal=='1')//boxing
		{
			state=ex_setup;
		}
		else if(kpVal=='2')//Situp
		{
			state=ex_setup;
		}
		else if(kpVal=='3')//jumping jacks
		{
			state=ex_setup;
		}
		else if(kpVal=='4')//pushup
		{
			state = ex_setup;
		}
		break;
		

		case ex_setup:
		// 		if(kpVal=='1')//boxing
		// 		{
		// 			state=boxingL;
		// 		}
		// 		else if(kpVal=='2')//Situp
		// 		{
		// 			state=su_UP;
		// 		}
		// 		else if(kpVal=='3')//jumping jacks
		// 		{
		// 			state=JJ_expand;
		// 		}
		// 		else if(kpVal=='4')//pushup
		// 		{
		// 			state = pushups_UP;
		// 		}
		state = ex_setup_isready;
		break;
		
		case ex_setup_isready:
		
		if(kpVal=='1')//boxing
		{
			state_delay++;
			if(state_delay >= 50)
			{
				state=boxingL;
				state_delay = 0;
			}
		}
		else if(kpVal=='2')//Situp
		{
			state_delay++;
			if(state_delay >= 50)
			{
				state=su_UP;
				state_delay = 0;
			}
		}
		else if(kpVal=='3')//jumping jacks
		{
			state_delay++;
			if(state_delay >= 50)
			{
				state=JJ_expand;
				state_delay = 0;
			}
		}
		else if(kpVal=='4')//pushup
		{
			state_delay++;
			if(state_delay >= 50)
			{
				state = pushups_UP;
				state_delay = 0;
			}
		}
		break;
		
		case boxingL:

		if(receive1 == 0x01)
		{
			// 			state_delay++;
			receive1 = 0x00;
			state = boxingR;
		}

		break;
		
		case boxingR:
		if(receive2 == 0x01)
		{
			state_delay++;
			receive2 = 0x00;
			state = boxingL;
			
			if(state_delay>=3)
			{
				state=boxingHL;
				state_delay=0;
			}
		}


		break;
		
		case boxingHL:
		if(receive1 == 0x01)
		{
			receive1 = 0x00;
			state = boxingHR;
		}
		break;
		
		case boxingHR:
		if(receive2 == 0x01)
		{
			state_delay++;
			receive2 = 0x00;
			state = boxingHL;
			
			if(state_delay>=3)
			{
				state=boxingUL;
				state_delay=0;
			}
		}
		break;
		
		case boxingUL:
		if(receive1 == 0x01)
		{
			receive1 = 0x00;
			state=boxingUR;
		}
		
		break;
		
		case boxingUR:
		if(receive2 == 0x01)
		{
			state_delay++;
			receive2 = 0x00;
			state=boxingUL;
			if(state_delay>=3)
			{
				state=finish1;
				state_delay=0;
			}
		}
		break;
		
		case su_UP:
		if(receive1 == 0x01)
		{
// 			receive1 = 0x00;
			state=su_DOWN;
		}
		
		break;
		
		case su_DOWN:
		if(receive1 == 0x00)
		{
			state_delay++;
// 			receive2 = 0x00;
			state=su_UP;
			if(state_delay>=3)
			{
				state=finish1;
				state_delay=0;
			}
		}
		break;
		
		case su_L_UP:
		if(receive1 == 0x01)
		{
			receive1 = 0x00;
			state=su_L_DOWN;
		}
		break;
		
		case su_L_DOWN:
		if(receive2 == 0x01)
		{
			state_delay++;
			receive2 = 0x00;
			state=su_L_UP;
			if(state_delay>=3)
			{
				state=finish1;
				state_delay=0;
			}
		}
		break;
		
		case su_R_UP:
		if(receive1 == 0x01)
		{
			receive1 = 0x00;
			state=su_R_DOWN;
		}
		break;
		
		case su_R_DOWN:
		if(receive2 == 0x01)
		{
			state_delay++;
			receive2 = 0x00;
			state=su_R_UP;
			if(state_delay>=3)
			{
				state=finish1;
				state_delay=0;
			}
		}
		break;
		
		case JJ_expand:
		if(receive1 == 0x01)
		{
// 			receive1 = 0x00;
			state=JJ_close;
		}
		break;
		
		case JJ_close:
		if(receive1 == 0x00)
		{
			state_delay++;
// 			receive2 = 0x00;
			state=JJ_expand;
			if(state_delay>=3)
			{
				state=finish1;
				state_delay=0;
			}
		}
		break;
		
		case pushups_UP:
		if(receive1 == 0x01)
		{
			state_delay++;
			receive1 = 0x00;
			if(state_delay >=10)
			{
				state = pushups_DOWN;
// 				state_delay=0;
			}
		}
		
		break;
		
		case pushups_DOWN:
		if(receive1 == 0x01)
		{
			state_delay++;
			receive1 = 0x00;
			if(state_delay >=10)
			{
				state = pushups_UP;
// 				state_delay=0;
			}
			if(state_delay >= 120)
			{
				state = finish1;
			}
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
		
		// 		case weight:
		// 		if(rightweight==10)
		// 		{
		// 			state=boxingL;
		// 		}
		// 		else
		// 		state=weight;
		// 		break;
		
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
		// 	state_delay = 0;//timing count for delay
		// 	 kpVal=0x00;//Keypad value
		// 	age_1=0x00; //first digit of age (1's place)
		// 	 age_2=0x00; //second digit of age (10's place)
		// 	 age_3=0x00; //third digit of age (100's place)
		// 	 age_4=0x00;
		// 	 total_age = 0x00; //Numerical age value as a whole
		// // 	static enum gen{male,female} gen_var;
		// 	 	gen_var = 0x00;
		// 	 feet = 0x00;
		// 	 inches1 = 0x00;
		// 	 inches2 =0x00;
		// 	 inches3 = 0x00;
		// 	 inches_total = 0x00;//Numerical inches total as a whole
		// 	 bWeight1 = 0x00;//body weight 1st digit
		// 	 bWeight2 = 0x00;//body weight 2nd digit
		// 	 bWeight3 = 0x00;//body weight 3rd digit
		// 	 bWeight4 = 0x00;//extra variable for '#'
		// 	 bWeight_total = 0x00;//Numerical total body weight combining 3 different digits ( bWeight1, bWeight2, bWeight3)
		// 	db_weight1 = 0x00;//first digit of weight in pounds
		// 	 db_weight2 = 0x00;//second digit of weight in pounds
		// 	db_weight3 = 0x00;//extra variable for '#' button
		// 	db_weight_total = 0x00;//Numerical total value of dumb-bell weights
		// 	 rightweight=0;
		// // 	static unsigned short x;
		// 	//my_char;
		// 	receive1 = 0x00;
		// 	receive2 = 0x00;
		// // 	receive3 = 0x00;
		// // 	receive4 = 0x00;
		//
		// 	 cals_burned = 0;
		break;
		case age1:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("Enter age:",1);
		nokia_lcd_render();
		age_1= GetKeypadKey();
		kpVal=age_1;
		break;
		
		case age1_press:
		nokia_lcd_set_cursor(70, 0);
		if(age_1 =='1')
		{
			nokia_lcd_write_string("1",1);
		}
		else if(age_1 =='2')
		{
			nokia_lcd_write_string("2",1);
		}
		else if(age_1 =='3')
		{
			nokia_lcd_write_string("3",1);
		}
		else if(age_1 =='4')
		{
			nokia_lcd_write_string("4",1);
		}
		else if(age_1 =='5')
		{
			nokia_lcd_write_string("5",1);
		}
		else if(age_1 =='6')
		{
			nokia_lcd_write_string("6",1);
		}
		else if(age_1 =='7')
		{
			nokia_lcd_write_string("7",1);
		}
		else if(age_1 =='8')
		{
			nokia_lcd_write_string("8",1);
		}
		else if(age_1 =='9')
		{
			nokia_lcd_write_string("9",1);
		}
		nokia_lcd_render();
		kpVal=GetKeypadKey();
		break;
		
		case age2:
		age_2= GetKeypadKey();
		kpVal=age_2;
		break;
		
		case age2_press:
		nokia_lcd_set_cursor(80, 0);
		if(age_2 =='1')
		{
			nokia_lcd_write_string("1",1);
		}
		else if(age_2 =='2')
		{
			nokia_lcd_write_string("2",1);
		}
		else if(age_2 =='3')
		{
			nokia_lcd_write_string("3",1);
		}
		else if(age_2 =='4')
		{
			nokia_lcd_write_string("4",1);
		}
		else if(age_2 =='5')
		{
			nokia_lcd_write_string("5",1);
		}
		else if(age_2 =='6')
		{
			nokia_lcd_write_string("6",1);
		}
		else if(age_2 =='7')
		{
			nokia_lcd_write_string("7",1);
		}
		else if(age_2 =='8')
		{
			nokia_lcd_write_string("8",1);
		}
		else if(age_2 =='9')
		{
			nokia_lcd_write_string("9",1);
		}
		nokia_lcd_render();
		kpVal=GetKeypadKey();
		break;
		
		case age3:
		age_3= GetKeypadKey();
		kpVal=age_3;
		break;
		
		case age3_press:
		nokia_lcd_set_cursor(80, 0);
		if(age_3 =='1')
		{
			nokia_lcd_write_string("1",1);
		}
		else if(age_3 =='2')
		{
			nokia_lcd_write_string("2",1);
		}
		else if(age_3 =='3')
		{
			nokia_lcd_write_string("3",1);
		}
		else if(age_3 =='4')
		{
			nokia_lcd_write_string("4",1);
		}
		else if(age_3 =='5')
		{
			nokia_lcd_write_string("5",1);
		}
		else if(age_3 =='6')
		{
			nokia_lcd_write_string("6",1);
		}
		else if(age_3 =='7')
		{
			nokia_lcd_write_string("7",1);
		}
		else if(age_3 =='8')
		{
			nokia_lcd_write_string("8",1);
		}
		else if(age_3 =='9')
		{
			nokia_lcd_write_string("9",1);
		}
		nokia_lcd_render();
		kpVal=GetKeypadKey();
		break;
		
		case age4:
		//variables age_1, age_2, age_3... need to append these three digits into a single number

		kpVal=GetKeypadKey();
		age_4 = kpVal;
		break;
		
		case age4_press:
		kpVal = GetKeypadKey();
		//========Calculations for creating age value==========
		if(age_2 == '#')//if there's only 1 digit
		{
			// 				age_1 = age_1 - 48;
			total_age = age_1-48;
		}
		else if(age_3 == '#')//if there's two digits
		{
			// 				age_1 = age_1 - 48;
			// 				age_2 = age_2 - 48;
			// 				age_1 = age_1 * 10;
			total_age = ((age_1-48)*10) + (age_2-48);
		}
		else if(age_4 == '#')//if there's three digits for age
		{
			// 				age_1 = age_1 - 48;
			// 				age_2 = age_2 - 48;
			// 				age_3 = age_3 - 48;
			// 				age_2 = age_2 * 10;
			// 				age_1 = age_1 * 100;
			total_age = ((age_1-48)*100) + ((age_2-48)*10) + (age_3-48);
		}
		break;
		
		
		case gender:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("Male: A",1);
		nokia_lcd_set_cursor(0,10);
		nokia_lcd_write_string("Female: B",1);
		nokia_lcd_render();
		kpVal=GetKeypadKey();
		if(kpVal=='A')
		{
			gen_var=male;
		}
		else if(kpVal=='B')
		{
			gen_var=female;
		}
		break;
		
		case gender_press:
		kpVal=GetKeypadKey();
		break;
		
		case height_ft:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("Enter height:",1);
		nokia_lcd_set_cursor(0,10);
		nokia_lcd_write_string("Feet: ",1);
		nokia_lcd_render();
		kpVal = GetKeypadKey();
		feet = kpVal;
		break;
		
		case height_ft_press:
		kpVal = GetKeypadKey();
		nokia_lcd_set_cursor(60,10);
		nokia_lcd_write_char(feet,1);
		nokia_lcd_render();
		
		break;
		
		case height_in1:
		nokia_lcd_set_cursor(0,20);
		nokia_lcd_write_string("inches: ",1);
		nokia_lcd_render();
		kpVal = GetKeypadKey();
		inches1 = kpVal;
		break;
		
		case height_in1_press:
		kpVal = GetKeypadKey();
		nokia_lcd_set_cursor(60,20);
		nokia_lcd_write_char(inches1,1);
		nokia_lcd_render();
		// 		if(inches1 == '#')
		// 		{
		// 			inches_total = 0x00;
		// 		}
		break;
		
		case height_in2:
		kpVal = GetKeypadKey();
		inches2 = kpVal;
		break;
		
		case height_in2_press:
		kpVal = GetKeypadKey();
		nokia_lcd_set_cursor(66,20);
		nokia_lcd_write_char(inches2,1);
		nokia_lcd_render();
		// 		if(inches2 == '#')
		// 		{
		// 			inches_total = inches1 - 48;//inches_total is now the actual decimal value
		// 		}
		
		break;
		
		case height_in3:
		kpVal = GetKeypadKey();
		inches3 = kpVal;
		break;
		
		case height_in3_press:
		kpVal = GetKeypadKey();
		
		if(inches1 == '#')
		{
			inches_total = 0x00;
		}
		else if(inches2 == '#')
		{
			inches_total = inches1 - 48;//inches_total is now the actual decimal value
		}
		else if(inches3 == '#')
		{
			// 			inches1 = inches1-48;
			// 			inches1 = inches1*10;
			// 			inches2 = inches2-48;
			inches_total = ((inches1-48)*10) + (inches2-48);
		}
		break;
		
		case bodyweight1:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("Body weight in lbs:",1);
		nokia_lcd_render();
		kpVal= GetKeypadKey();
		bWeight1 = kpVal;
		break;
		
		case bodyweight1_press:
		kpVal = GetKeypadKey();
		nokia_lcd_set_cursor(0,30);
		nokia_lcd_write_char(bWeight1,1);
		nokia_lcd_render();
		break;
		
		case bodyweight2:
		kpVal= GetKeypadKey();
		bWeight2 = kpVal;
		break;
		
		case bodyweight2_press:
		kpVal = GetKeypadKey();
		nokia_lcd_set_cursor(10,30);
		nokia_lcd_write_char(bWeight2,1);
		nokia_lcd_render();
		break;
		
		case bodyweight3:
		kpVal = GetKeypadKey();
		bWeight3 = kpVal;
		break;
		
		case bodyweight3_press:
		kpVal = GetKeypadKey();
		nokia_lcd_set_cursor(20,30);
		nokia_lcd_write_char(bWeight3,1);
		nokia_lcd_render();
		break;
		
		case bodyweight4:
		kpVal = GetKeypadKey();
		bWeight4 = kpVal;
		break;
		
		case bodyweight4_press:
		kpVal = GetKeypadKey();
		if(bWeight2 == '#')
		{
			// 				bWeight1 = bWeight1 - 48;
			bWeight_total = bWeight1-48;
		}
		else if(bWeight3 == '#')//if only 2 digits were entered
		{
			// 				bWeight1 = bWeight1 - 48;//turn these chars into their actual numeric values
			// 				bWeight2 = bWeight2 - 48;//turn these chars into their actual numeric values
			// 				bWeight1 = bWeight1 * 10;//in values less than 100, bWeight1 is in the 10's place
			bWeight_total = ((bWeight1-48)*10) + (bWeight2-48);
		}
		else if(bWeight4 == '#')//if 3 digits were entered
		{
			// 				bWeight1 = bWeight1 - 48;
			// 				bWeight2 = bWeight2 - 48;
			// 				bWeight3 = bWeight3 - 48;
			// 				bWeight2 = bWeight2 * 10;
			// 				bWeight1 = bWeight1 * 100;
			bWeight_total = ((bWeight1-48)*100) + ((bWeight2-48)*10) + (bWeight3-48);
		}
		
		break;
		
		case dumb_bell_weight1:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("Weight of dumbbells:",1);
		nokia_lcd_render();
		kpVal= GetKeypadKey();
		db_weight1 = kpVal;
		break;
		
		case dumb_bell_weight1_press:
		kpVal = GetKeypadKey();
		nokia_lcd_set_cursor(0,30);
		nokia_lcd_write_char(db_weight1,1);
		nokia_lcd_render();
		break;
		
		case dumb_bell_weight2:
		kpVal = GetKeypadKey();
		db_weight2 = kpVal;
		break;
		
		case dumb_bell_weight2_press:
		kpVal = GetKeypadKey();
		nokia_lcd_set_cursor(10,30);
		nokia_lcd_write_char(db_weight2,1);
		nokia_lcd_render();
		break;
		
		case dumb_bell_weight3:
		kpVal = GetKeypadKey();
		db_weight3 = kpVal;
		break;
		
		case dumb_bell_weight3_press:
		kpVal = GetKeypadKey();
		if(db_weight1 == '#')
		{
			db_weight_total = 0x00;
		}
		else if(db_weight2 == '#')
		{
			db_weight_total = db_weight1 - 48;
		}
		else if(db_weight3 == '#')
		{
			db_weight_total = ((db_weight1-48)*10) + (db_weight2-48);
		}
		break;
		
		case ex_selection:
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
		
		case ex_setup:
		
		USART_Flush(0);
		if(USART_IsSendReady(0))
		USART_Send(kpVal,0);
		while(!USART_HasTransmitted(0)){
			//wait until transmitted
		}
		USART_Flush(1);
		if(USART_IsSendReady(1))
		USART_Send(kpVal,1);
		while(!USART_HasTransmitted(1)){
			//wait until transmitted
		}
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("Get Ready",2);
		nokia_lcd_render();

		break;
		
		case ex_setup_isready://telling the statemachine which state to go to

		break;
		
		case boxingL:

		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("LEFT", 2);
		nokia_lcd_set_cursor(0,30);
		nokia_lcd_write_string("JAB",2);
		nokia_lcd_render();
		// 		USART_Flush(0);
		// 		if(USART_IsSendReady(0))
		// 		USART_Send(kpVal,0);
		// 		while(!USART_HasTransmitted(0)){
		// 			//wait until transmitted
		// 		}
		// 		USART_Flush(1);
		// 		if(USART_IsSendReady(1))
		// 		USART_Send(0x00,1);
		// 		while(!USART_HasTransmitted(1)){
		// 			//wait until transmitted
		// 		}
		if(USART_HasReceived(0))
		{
			PORTA = 0x03;
			receive1 = USART_Receive(0);
			USART_Flush(0);
		}
		PORTA = 0x01;
		break;
		
		case boxingR:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("RIGHT", 2);
		nokia_lcd_set_cursor(0,30);
		nokia_lcd_write_string("JAB",2);
		nokia_lcd_render();
		// 		USART_Flush(0);
		// 		if(USART_IsSendReady(0))
		// 		USART_Send(0x00,0);
		// 		while(!USART_HasTransmitted(0)){
		// 			//wait until transmitted
		// 		}
		// 		USART_Flush(1);
		// 		if(USART_IsSendReady(1))
		// 		USART_Send(kpVal,1);
		// 		while(!USART_HasTransmitted(1)){
		// 			//wait until transmitted
		// 		}

		if(USART_HasReceived(1))
		{
			receive2 = USART_Receive(1);
			USART_Flush(1);
		}
		break;
		
		case boxingHL:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("LEFT", 2);
		nokia_lcd_set_cursor(0,30);
		nokia_lcd_write_string("HOOK",2);
		nokia_lcd_render();
		if(USART_HasReceived(0))
		{
			receive1 = USART_Receive(0);
			USART_Flush(0);
		}
		break;
		
		case boxingHR:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("RIGHT", 2);
		nokia_lcd_set_cursor(0,30);
		nokia_lcd_write_string("HOOK",2);
		nokia_lcd_render();
		if(USART_HasReceived(1))
		{
			receive2 = USART_Receive(1);
			USART_Flush(1);
		}
		break;
		
		case boxingUL:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("LEFT", 2);
		nokia_lcd_set_cursor(0,30);
		nokia_lcd_write_string("UPPERCUT",2);
		nokia_lcd_render();
		if(USART_HasReceived(0))
		{
			receive1 = USART_Receive(0);
			USART_Flush(0);
		}
		break;
		
		case boxingUR:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("RIGHT", 2);
		nokia_lcd_set_cursor(0,30);
		nokia_lcd_write_string("UPPERCUT",2);
		nokia_lcd_render();
		if(USART_HasReceived(1))
		{
			receive2 = USART_Receive(1);
			USART_Flush(1);
		}
		break;
		
		case su_UP:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("SIT", 2);
		nokia_lcd_set_cursor(0,30);
		nokia_lcd_write_string("UP",2);
		nokia_lcd_render();
		// 		USART_Flush(0);
		// 		if(USART_IsSendReady(0))
		// 		USART_Send(kpVal,0);
		// 		while(!USART_HasTransmitted(0)){
		// 			//wait until transmitted
		// 		}
		// 		USART_Flush(1);
		// 		if(USART_IsSendReady(1))
		// 		USART_Send(0x00,1);
		// 		while(!USART_HasTransmitted(1)){
		// 			//wait until transmitted
		// 		}
		if(USART_HasReceived(0))
		{
			PORTA = 0x03;
			receive1 = USART_Receive(0);
			USART_Flush(0);
		}
		PORTA = 0x01;
		break;
		
		case su_DOWN:
				nokia_lcd_clear();
				nokia_lcd_set_cursor(0, 0);
				nokia_lcd_write_string("SIT", 2);
				nokia_lcd_set_cursor(0,30);
				nokia_lcd_write_string("DOWN",2);
				nokia_lcd_render();
				// 		USART_Flush(0);
				// 		if(USART_IsSendReady(0))
				// 		USART_Send(kpVal,0);
				// 		while(!USART_HasTransmitted(0)){
				// 			//wait until transmitted
				// 		}
				// 		USART_Flush(1);
				// 		if(USART_IsSendReady(1))
				// 		USART_Send(0x00,1);
				// 		while(!USART_HasTransmitted(1)){
				// 			//wait until transmitted
				// 		}
				if(USART_HasReceived(0))
				{
					PORTA = 0x03;
					receive1 = USART_Receive(0);
					USART_Flush(0);
				}
				PORTA = 0x01;
		break;
		
		case su_L_UP:
		break;
		
		case su_L_DOWN:
		break;
		
		case su_R_UP:
		break;
		
		case su_R_DOWN:
		break;
		
		case JJ_expand:
				nokia_lcd_clear();
				nokia_lcd_set_cursor(0, 0);
				nokia_lcd_write_string("EXPAND", 2);
				nokia_lcd_render();
				// 		USART_Flush(0);
				// 		if(USART_IsSendReady(0))
				// 		USART_Send(kpVal,0);
				// 		while(!USART_HasTransmitted(0)){
				// 			//wait until transmitted
				// 		}
				// 		USART_Flush(1);
				// 		if(USART_IsSendReady(1))
				// 		USART_Send(0x00,1);
				// 		while(!USART_HasTransmitted(1)){
				// 			//wait until transmitted
				// 		}
				if(USART_HasReceived(0))
				{
					PORTA = 0x03;
					receive1 = USART_Receive(0);
					USART_Flush(0);
				}
				PORTA = 0x01;
		break;
		
		case JJ_close:
						nokia_lcd_clear();
						nokia_lcd_set_cursor(0, 0);
						nokia_lcd_write_string("CLOSE", 2);
						nokia_lcd_render();
						// 		USART_Flush(0);
						// 		if(USART_IsSendReady(0))
						// 		USART_Send(kpVal,0);
						// 		while(!USART_HasTransmitted(0)){
						// 			//wait until transmitted
						// 		}
						// 		USART_Flush(1);
						// 		if(USART_IsSendReady(1))
						// 		USART_Send(0x00,1);
						// 		while(!USART_HasTransmitted(1)){
						// 			//wait until transmitted
						// 		}
						if(USART_HasReceived(0))
						{
							PORTA = 0x03;
							receive1 = USART_Receive(0);
							USART_Flush(0);
						}
						PORTA = 0x01;
		break;
		
		case pushups_UP:
						nokia_lcd_clear();
						nokia_lcd_set_cursor(0, 0);
						nokia_lcd_write_string("UP", 3);
						nokia_lcd_render();
						// 		USART_Flush(0);
						// 		if(USART_IsSendReady(0))
						// 		USART_Send(kpVal,0);
						// 		while(!USART_HasTransmitted(0)){
						// 			//wait until transmitted
						// 		}
						// 		USART_Flush(1);
						// 		if(USART_IsSendReady(1))
						// 		USART_Send(0x00,1);
						// 		while(!USART_HasTransmitted(1)){
						// 			//wait until transmitted
						// 		}
						if(USART_HasReceived(0))
						{
							PORTA = 0x03;
							receive1 = USART_Receive(0);
							USART_Flush(0);
						}
						PORTA = 0x01;
		break;
		
		case pushups_DOWN:
						nokia_lcd_clear();
						nokia_lcd_set_cursor(0, 0);
						nokia_lcd_write_string("DOWN", 2);
						nokia_lcd_render();
						// 		USART_Flush(0);
						// 		if(USART_IsSendReady(0))
						// 		USART_Send(kpVal,0);
						// 		while(!USART_HasTransmitted(0)){
						// 			//wait until transmitted
						// 		}
						// 		USART_Flush(1);
						// 		if(USART_IsSendReady(1))
						// 		USART_Send(0x00,1);
						// 		while(!USART_HasTransmitted(1)){
						// 			//wait until transmitted
						// 		}
						if(USART_HasReceived(0))
						{
							PORTA = 0x03;
							receive1 = USART_Receive(0);
							USART_Flush(0);
						}
						PORTA = 0x01;
		break;
		
		case finish1:
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("GREAT", 2);
		nokia_lcd_set_cursor(0,25);
		nokia_lcd_write_string("WORKOUT",2);
		nokia_lcd_render();
		rightweight = db_weight_total + bWeight_total;
		rightweight = rightweight/2.2;
		cals_burned = 0.08 * rightweight * 5;
		
		break;
		
		case finish2:
		dtostrf( cals_burned, 3, 2, float_ );
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("BURNED CALORIES:", 1);
		nokia_lcd_set_cursor(0,15);
		nokia_lcd_write_string(float_,2);
		nokia_lcd_render();
		break;
		
		// 		case weight:
		//
		// 		// 				x = ADC;
		// 		// 				my_char = (char)x;
		// 		// 				PORTD = my_char;
		// 		// //				my_char=(char)(x>>8);
		// 		break;
		
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
	//DDRD = 0xFF;//output to measure weight
	//PORTD = 0x00;
	DDRC = 0xF0;//PORT for Keypad
	PORTC = 0xF0;//initialize port
	DDRB = 0xFF;//Nokia5110
	PORTB = 0x00;
	DDRA = 0xFF;//output showing that the bluetooth and hand devices should be on
	PORTA=0x00;
	//ADC_init();
	initUSART(0);//0 is for left hand
	initUSART(1);//1 is for right hand
	
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

	TimerSet(100);
	TimerOn();
	
	while(1)
	{

		
	}

}

