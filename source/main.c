/*	Author: Raymond Booth
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment:	Final 
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "timer.h"
#include "scheduler.h"
#include "io.h"
#include <stdlib.h>
#include <time.h>
//-------Shared variables--------//
unsigned int score = 0;
unsigned char wins = 1;
unsigned char cursor = 1;

int ob1 = 10;
int ob2 = 20;
unsigned int timer= 0;
unsigned int check = 25;
unsigned char yes = 0x00;

unsigned char projectile = -1 ;
unsigned char hit = 0x00;

//----End of shared variables----//

enum States{one,two,three,four, wait, wait1,wait22,wait33,over};
//This function controls the movment of the character
int LCD(int state)
{
	unsigned char input = ~PINA & 0x0F;
	switch(state)
	{
		case one:
			if(input == 0x02)
				state = wait;
			else if(input == 0x04)
			{
				if((cursor > 0 && cursor < 16) || (cursor < 32 && cursor > 17))
					cursor++;
				state = wait22;
			}
			else if (input == 0x08)
			{
				if((cursor > 1 && cursor < 17) || (cursor < 33 && cursor > 18))
					cursor--;
				state = wait33;	
			}
			else
				state = one;
			if(wins == 1)
			{
				LCD_ClearScreen();
				state = over;
			}
			break;
		case wait:
			if(cursor <= 16)
			cursor += 16;
			LCD_Cursor(cursor);
			if(input == 0x00)
				state = two;
			else
				state = wait;
			if(wins == 1)
			{
				LCD_ClearScreen();
				state = over;
			}
			break;
		case two:
			if(input == 0x01)
				state = wait1;
			else if(input == 0x04)
			{
				if((cursor > 0 && cursor < 16) || (cursor < 32 && cursor > 17))
					cursor++;
				state = wait22;
			}
			else if (input == 0x08)
			{
				if((cursor > 1 && cursor < 17) || (cursor < 33 && cursor > 18))
					cursor--;
				state = wait33;	
			}
			else
				state = two;
			if(wins == 1)
			{
				LCD_ClearScreen();
				state = over;
			}
			break;
		case wait1:
			if(cursor > 16)
				cursor -= 16;
			LCD_Cursor(cursor);
			if(input == 0x00)
				state = one;
			else
				state = wait1;
			if(wins == 1)
			{
				LCD_ClearScreen;
				state = over;
			}
			break;
		case three:
			if(input == 0x01)
				state = wait1;
			else if(input == 0x02)
				state = wait;
			else if (input == 0x08)
			{
				if((cursor > 1 && cursor < 17) || (cursor < 33 && cursor > 18))
					cursor--;
				state = wait33;	
			}
			else if (input == 0x04)
			{
				if((cursor > 0 && cursor < 16) || (cursor < 32 && cursor > 17))
					cursor++;
				state = wait22;	
			}
			else
				state = three;
			if(wins == 1)
			{
				LCD_ClearScreen();
				state = over;
			}
			break;
		case wait22:
			LCD_Cursor(cursor);
			if(input == 0x00)
				state = three;
			else
				state = wait22;
			
			break;
		case four:
			if(input == 0x01)
				state = wait1;
			else if(input == 0x02)
				state = wait;
			else if (input == 0x04)
			{
				if((cursor > 0 && cursor < 16) || (cursor < 32 && cursor > 17))
					cursor++;
				state = wait22;	
			}
			else if (input == 0x08)
			{
				if((cursor > 1 && cursor < 17) || (cursor < 33 && cursor > 18))
					cursor--;
				state = wait33;	
			}
			else
				state = three;
			if(wins == 1)
			{
				LCD_ClearScreen();
				state = over;
			}
			break;
		case wait33:
			LCD_Cursor(cursor);
			if(input == 0x00)
				state = four;
			else
				state = wait33;
			
			break;
		case over:
			if(wins == 0)
				state = one;
			else
				state = over;
			break;
		default :
			state = one;
			break;
	}
	return state;
}
unsigned char p2 = 0; //dictates if 2 player game was selectd
unsigned char p2c = 16;//the player 2 position
enum{enable,up,down,upWait,downWait};
int player2(int state)
{
	unsigned char input = ~PINA & 0xC0;
	switch(state)
	{
		case enable:
			if(p2 == 1)
				state = up;
			break;
		case up:
			p2c = 16;
			LCD_Cursor(p2c);
			LCD_WriteData('<');
			LCD_Cursor(cursor);
			if(input == 0x80)
				state = upWait;
			else 
				state = up;
			break;
		case upWait:
			if(input == 0x00)
				state = down;
			else
				state = upWait;
			break;
		case down:
			p2c = 32;
			LCD_Cursor(p2c);
			LCD_WriteData('<');
			LCD_Cursor(cursor);
			if(input == 0x40)
				state = downWait;
			else
				state = down;
			break;
		case downWait:
			if(input == 0x00)
				state = up;
			else
				state = downWait;
			break;
	}

}
unsigned char bullet = 16;
enum {go, fly, length};
int p2weapon(int state)
{
	unsigned char input = ~PINA & 0x10;
	switch(state)
	{
		case go:
			if(p2 == 1)
				state = fly;
			else 
				state = go;
			break;
		case fly:
			LCD_Cursor(bullet);
			LCD_WriteData('-');
			LCD_Cursor(cursor);
			if(bullet == 1 || bullet == 17)
				bullet = p2c;//brings bullet back to shoot again
			state = fly;
			bullet--;
			if(wins == 1)
				state = length;
			break;
		case length:
			if(input == 0x10)
				state = fly;
			else 
				state = length;
			break;
	}
}


enum wepStates {trigger, launch, duration};
//This SM controlls the projectile that can destroy some objects
int weapon(int state)
{

	unsigned char input = ~PINA & 0x20;
	switch (state)
	{
		case trigger:
			if(input == 0x20 && wins == 0)
				state = launch;
			else
				state = trigger;
			break;
		case launch:
			if(input == 0x00)
			{
				projectile = cursor;
				state = duration;
			}
			else
				state = launch;
			break;
		case duration:
			LCD_Cursor(projectile);
			LCD_WriteData('-');
			LCD_Cursor(cursor);
			if(projectile == ob1 ||projectile == ob2)
			{
					
				if(projectile == ob2)
					ob2 = 0;
				state = trigger;
				hit == 1;
				projectile = -1;
			}
			else if(projectile == 16 || projectile == 32)
			{
				projectile = -1;
				state = trigger;
			}
			else
				state = duration;
			projectile++;
			break;
		default: 
			state = trigger;
			break;
	}
	return state;
}
enum obsStates {menu1,start,wait2,move,wait3,over1,wait4};
//This funciton handles the movement of the obsticals as well
//as the starting and stopping of the game
int obs(int state)
{
	int enable = rand() % 10; //Makes it so the object respond randomly
	int enable1 = rand() % 10;
	unsigned char input = ~PINA & 0x10;
	switch(state)
	{
		case menu1:
			if(wins == 0)
				state = start;
			break;
		
		case start:
			if(input == 0x10)
				state = wait2;
			else
				state = start;
			if(wins == 1)
			{
				state = over1;
			}
			LCD_ClearScreen();
			LCD_Cursor(ob1);
			LCD_WriteData('#');
			LCD_Cursor(ob2);
			LCD_WriteData('*');
			LCD_Cursor(cursor);
			
			break;
		case wait2:
			if(input == 0x00)
			{
				state = move;
			}
			else
				state = wait2;
			break;
		case move:
			timer++;
			if(timer >= check && check < 100)
			{
				timer = 0;
				check *= 2;
				yes = 1;
			}
			//unbreakable obj
			if(ob1 < 1)
			{
				hit = 0;
				if(enable == 1)
				{
					score++;
					ob1 = 16;
				}
			}
			//breakable obj
			if(ob2 < 18 || hit == 1)
			{
				hit = 0;
				if(enable1 == 1)
				{
					score++;
					ob2 = 32;
				}
			}
			ob1 --;
			ob2 --;
			LCD_ClearScreen();
			LCD_Cursor(ob1);
			LCD_WriteData('#');
			LCD_Cursor(ob2);
			LCD_WriteData('*');
			LCD_Cursor(cursor);
			if(input == 0x10)
				state = wait3;
			else
				state = move;
			if(wins == 1)
				state = over1;
			break;
		case wait3:
			if(input == 0x00)
			{
				state = start;
			}
			else
				state = wait3;
			break;
		case over1:
			if(input == 0x10)
			{
				wins = 0;
				state = wait4;
				LCD_ClearScreen();
			}
			else
				state = over1;
			break;
		case wait4:
			if(input == 0x00)
				state = move;
			else
				state = wait4;
			break;
		default: 
			state = start;
			break;
	}

	return state;
}
unsigned char reset = 0x00;
enum winState{ gfix,menu,menuWait, Start,crash, end};
//This SM handles the detections of an obstical hitting the player
int win(int state)
{
	unsigned char input = ~PINA & 0x03;
	char sco[3];
	sprintf(sco, "%d", score);
	static unsigned int count;
	switch(state)
	{
		case gfix:
			wins = 1;
			LCD_ClearScreen();
			LCD_DisplayString(1, "UP 1P, down 2P");		
			state = menu;
			break;
		case menu:
			if(input == 0x01)//starts the single player mode
				state = menuWait;
			else if(input == 0x02)//starts the 2 player mode
			{
				state = menuWait;
				p2 = 1;
			}
			break;
		case menuWait:
			if(input == 0x00)
			{
				wins = 0;
				LCD_ClearScreen();
				state = Start;
			}
			else 
				state = menuWait;
			break;
		case Start:
			state = Start;
			if(cursor == ob1 || cursor == ob2 || cursor == bullet)
			{
				reset = 1;
				count = 0;
				wins = 1;
				state = crash;
				LCD_DisplayString(cursor,"CRASH!");
			}
			break;
		case crash:
			if(count < 12)
			{
				count++;
			}
			else
			{
				LCD_ClearScreen();
				LCD_DisplayString(4, "Game Over        score:");
				LCD_Cursor(27);
				LCD_WriteData(sco[0]);
				LCD_Cursor(28);
				LCD_WriteData(sco[1]);
				state = end;
			}
			break;
		case end:
			if(wins == 0)
			{
				score = 0;
				check = 25;
				reset = 0;
				cursor = 1;
				ob1 = 10;
				ob2 = 20;
				bullet = p2c;
				state = Start;
			}
			break;
		default:
			state = Start;
			break;
	}
	return state;
}
int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	srand(time(NULL));//initialise random

	static task task1,task2,task3,task4,task5,task6;
	task *tasks[] = {&task1, &task2,&task3,&task4,&task5,&task6};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	//character SM
	task1.state = one;
	task1.period = 100;
	task1.elapsedTime = task1.period;
	task1.TickFct = &LCD;
	//Obsticals SM
	task2.state = menu1;
	task2.period = 200;
	task2.elapsedTime = task2.period;
	task2.TickFct = &obs;
	//Menu and end game SM
	task3.state = gfix;
	task3.period = 100;
	task3.elapsedTime = task3.period;
	task3.TickFct = &win;
	//weapon SM
	task4.state = trigger;
	task4.period = 50;
	task4.elapsedTime = task4.period;
	task4.TickFct = &weapon;
	//Player 2 SM
	task5.state = enable;
	task5.period = 100;
	task5.elapsedTime = task5.period;
	task5.TickFct = &player2;
	//Player 2 weapon SM
	task6.state = go;
	task6.period = 100;
	task6.elapsedTime = task6.period;
	task6.TickFct = &p2weapon; 
	//cal gcd
	
	unsigned long gcd = tasks[0]->period;
	for(int j = 0; j < numTasks; j++ )
	{
		gcd = findGCD(gcd,tasks[j]->period);
	}

	TimerSet(gcd);
	TimerOn();
	unsigned short i;
	LCD_init();
	
    while (1) {
	
	    for(i = 0; i < numTasks; i++)
	    {
		    if(tasks[i]->elapsedTime == tasks[i]->period)
		    {
			    tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			    tasks[i]->elapsedTime = 0;
		    }
		    tasks[i]->elapsedTime += gcd;
	    }
	    if(yes == 1)
	    {
		    yes = 0;
		    tasks[1]->period = tasks[1]->period / 2; 
	    }
	    if(reset == 1)
		    tasks[1]->period = 200;
	    while(!TimerFlag);
	    TimerFlag = 0;



    }
    return 1;
}
