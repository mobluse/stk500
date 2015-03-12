// File: hippocampusgame.c
/////////////////////////////////////////////////////////////////////
//
// Hippocampus Game by M.O.B for STK500.
// Copyright (C) 2008 by Mikael O. Bonnier, Lund, Sweden.
// All rights reserved. This program is free software licensed under 
// the terms of "GNU General Public License" v3 or later, see
// <http://www.gnu.org/copyleft/gpl.html>. Donations are welcome.
// The source code is at <http://www.df.lth.se.orbin.se/~mikaelb/micro/stk500/>.
//
// This program lets the user play Brain Game (a.k.a. Simon) using 
// LED0-LED3 and by pressing the switches SW0-SW3.
// The goal is to emulate the Velleman MK112.
//
// First press SW0 or SW1 to start and select sound or mute, respectively 
// (sound is currently not supported). Then press one of SW0-SW3 to select
// level. Higher level gives longer maximum sequence to repeat. The micro 
// controller plays a sequence. Repeat the sequence by pressing the switches
// under the leds. The sequence increased by one is then played. Repeat. 
// Eventually you will win, or make a mistake or timeout and loose. You will
// be celebrated or mocked, and the game is restarted, and you select sound 
// or mute again.
//
// Target platform is STK500 with Atmel AVR ATMEGA8515 micro controller 
// configured according to quick start in the manual, i.e. PORTB to LEDS 
// and PORTD to SWITCHES.
//
// It was initially developed in C using AVR Studio 4 with GCC from WinAVR.
// It can be compiled and programmed to chip in Linux using:
// $ avr-gcc -Os -std=c99 -mmcu=atmega8515 -o hippocampusgame.elf hippocampusgame.c
// $ avr-objcopy -j .text -j .data -O ihex hippocampusgame.elf hippocampusgame.hex
// $ avrdude -c stk500v2 -p m8515 -P /dev/ttyUSB0 -U flash:w:hippocampusgame.hex
//
// Road map:
// * port to other evaluation boards using other micro controllers
// * improve debouncing
// * implement sleep
// * make it interrupt driven
// * use more power saving features
// * implement sound
// * optimize.
//
// Revision history:
// 2008-Oct-11:     v.0.0.1
// 2008-Oct-12:     v.0.0.2
// 2008-Nov-14:     v.0.0.3
//
// Suggestions, improvements, and bug-reports
// are always welcome to:
//                  Mikael Bonnier
//                  Osten Undens gata 88
//                  SE-227 62  LUND
//                  SWEDEN
//
// Or use my electronic addresses:
//     Web: http://www.df.lth.se.orbin.se/~mikaelb/
//     E-mail/MSN: mikael.bonnier@gmail.com
//     ICQ # 114635318
//     Skype: mikael4u
//              _____
// :           /   / \           :
// ***********/   /   \***********
// :         /   /     \         :
// *********/   /       \*********
// :       /   /   / \   \       :
// *******/   /   /   \   \*******
// :     /   /   / \   \   \     :
// *****/   /   /***\   \   \*****
// :   /   /__ /_____\   \   \   :
// ***/               \   \   \***
// : /_________________\   \   \ :
// **\                      \  /**
// :  \______________________\/  :
//
// Mikael O. Bonnier
/////////////////////////////////////////////////////////////////////

#include <avr/io.h> // avr header file for IO ports
#include <stdlib.h> // random()

void init(void);
int play(void);
void celebrate(void);
void mock(void);
void gen_seq(int level);
int standby(void);
void while_key(unsigned char key);
void while_key_not(unsigned char key);
void while_keypattern_not(unsigned char keypattern);
int wait_for_key(unsigned char key);
int delay_long(int count, int interruptible);
int delay(int interruptible);
unsigned char number2bits(int number);
int bits2number(unsigned char bits);

int sleep = 1;
unsigned char keyboard;
unsigned long seq;
int seq_len;

// These constants can be adjusted
const int lengths[4] = {5, 7, 9, 11}; // Max length is 15
#define DELAYTIME 300
#define STANDBYTIMEOUT 20
#define KEYTIMEOUT 1000000


int main(void)
{
	DDRD = 0x00; // set PORTD for input
	DDRB = 0xFF; // set PORTB for output
	PORTB = 0xFF; // switch off all leds
	while_key(0b11111111); // wait for user to press a key

	for(;;)
	{
		init();
		if(sleep)
			continue;
		int win = play();
		if(win)
			celebrate();
		else
			mock();
		while_key_not(0b11111111);
	}
	return 1;
}

void init(void)
{
	while_keypattern_not(0b00000011); // wait for SW0 (sound) or SW1 (mute)
	if(sleep)
		return;
	PORTB = keyboard;
	while_key_not(0b11111111); // wait for release of all keys

	while_keypattern_not(0b00001111); // wait for level 0-3
	if(sleep)
		return;
	PORTB = keyboard;
	int level = bits2number(keyboard);
	while_key_not(0b11111111);
	PORTB = keyboard;
	gen_seq(level);
}

int play(void)
{
	for(int i = 1; i <= seq_len; ++i)
	{
		delay_long(12, 0);
		for(int j = 0; j < i; ++j)
		{
			int led = seq >> (j<<1) & 0b11;
			PORTB = number2bits(led);
			delay_long(4, 0);
			PORTB = 0b11111111;
			delay_long(4, 0);
		}
		for(int j = 0; j < i; ++j)
		{
			int error = 0;
			int led = seq >> (j<<1) & 0b11;
			error = wait_for_key(number2bits(led));
			if(error)
				return 0;
		}
		
	}
	return 1;
}

void celebrate(void)
{
	for(int i = 0; i < 3; ++i)
	{
		PORTB = 0b11111110;
		delay(0);
		PORTB = 0b11111111;
		delay(0);
		PORTB = 0b11111110;
		delay(0);
		PORTB = 0b11111111;
		delay(0);
		PORTB = 0b11111101;
		delay(0);
		PORTB = 0b11111111;
		delay(0);
		PORTB = 0b11111101;
		delay(0);
		PORTB = 0b11111111;
		delay(0);
		PORTB = 0b11111011;
		delay(0);
		PORTB = 0b11111111;
		delay(0);
		PORTB = 0b11111011;
		delay(0);
		PORTB = 0b11111111;
		delay(0);
		PORTB = 0b11110111;
		delay(0);
		PORTB = 0b11111111;
		delay(0);
		PORTB = 0b11110111;
		delay(0);
		PORTB = 0b11111111;
		delay(0);
	}
}

void mock(void)
{
	for(int i = 0; i < 15; ++i)
	{
		PORTB = 0xF0;
		delay(0);
		PORTB = 0xFF;
		delay(0);
	}
}

void gen_seq(int level)
{
	seq_len = lengths[level];
	seq = random();
}

int standby(void)
{
	PORTB = 0xFF;
	for(int count = 0; count < STANDBYTIMEOUT; ++count)
	{
		unsigned char leds = 0b11111110;
		for(int i = 0; i < 4; ++i)
		{
			if(!sleep)
				PORTB = leds;
			if(delay(1))
			{
				PORTB = 0xFF;
				return 0;
			}
			leds <<= 1;
			leds |= 1;
		}
	}
	PORTB = 0xFF;
	return 1;
}

void while_key(unsigned char key)
{
	do 
	{
		keyboard = PIND;
	} while(keyboard == key);
}

void while_key_not(unsigned char key)
{
	do 
	{
		keyboard = PIND;
	} while(keyboard != key);
}

void while_keypattern_not(unsigned char keypattern)
{
	do 
	{
		int timeout = standby();
		if(timeout)
		{
			sleep = 1;
			return;
		}
	} while(!(~keyboard & keypattern & (unsigned char)0xFF));
	sleep = 0;
}

int wait_for_key(unsigned char key)
{
	unsigned long count = 0;
	while_key_not(0b11111111);
	do 
	{
		if(count > KEYTIMEOUT)
			break;
		++count;
		keyboard = PIND;
	} while(keyboard == 0b11111111);
	PORTB = keyboard;
	int error = keyboard != key;
	while_key_not(0b11111111);
	PORTB = keyboard;
	return error;
}

int delay_long(int count, int interruptible)
{
	for(int i = 0; i < count; ++i)
		if(delay(interruptible))
			return 1;
	return 0;
}

int delay(int interruptible)
{
	for(int i = 0; i < DELAYTIME; ++i)
	{
		keyboard = PIND;
		if(interruptible && keyboard != 0b11111111) {
			PORTB = keyboard;
			return 1;
		}
		random();
	}
	return 0;
}

unsigned char number2bits(int number)
{
	return ~(0b1 << number);
}

int bits2number(unsigned char bits)
{
	switch(bits)
	{
		case 0b11111110:
			return 0;
		case 0b11111101:
			return 1;
		case 0b11111011:
			return 2;
		case 0b11110111:
			return 3;
	}
	return 8;
}
