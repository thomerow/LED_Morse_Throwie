/*
 * main.c
 *
 *  Created on: 24.04.2011
 *      Author: baum
 */
#include "main.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>


#define LENGTH_DIT 1				// Length of a "dit".
#define LENGTH_DAH (3 * LENGTH_DIT)		// Each "dah" equals 3 "dits". ;)
#define PAUSE_CHAR (3 * LENGTH_DIT)		// The pause between each letter is 3 "dits".
#define PAUSE_WORD (5 * LENGTH_DIT)		// The pause between each word is 5 "dits".
#define PAUSE_TEXT 40				// Pause before retransmitting text.


// The text (Resides in program space thanks to macros defined in avr/pgmspace.h
// (see http://www.nongnu.org/avr-libc/user-manual/pgmspace.html)). Must be lower case.
// For known characters see definitions below.

char theText[] PROGMEM = "dare deep";


volatile int 	nLenText 	= 0;
volatile int 	nLenChar 	= 0;
volatile int 	nLenSymb	= 0;
volatile int 	nLenPause 	= 0;
volatile int 	nPosText 	= 0;
volatile int 	nPosChar 	= 0;
volatile char 	chCur 		= 0;
volatile unsigned char uMorse = 0;


// The morse alphabet (0 = "dit", 1 = dah", the rightmost 1 denotes the end of a sequence)
const unsigned char letters[] = {
	0b01100000 /* a */, 0b10001000 /* b */, 0b10101000 /* c */, 0b10010000 /* d */,
	0b01000000 /* e */, 0b00101000 /* f */, 0b11010000 /* g */, 0b00001000 /* h */,
	0b00100000 /* i */, 0b01111000 /* j */, 0b10110000 /* k */, 0b01001000 /* l */,
	0b11100000 /* m */, 0b10100000 /* n */, 0b11110000 /* o */, 0b01101000 /* p */,
	0b11011000 /* q */, 0b01010000 /* r */, 0b00010000 /* s */, 0b11000000 /* t */,
	0b00110000 /* u */, 0b00011000 /* v */, 0b01110000 /* w */, 0b10011000 /* x */,
	0b10111000 /* y */, 0b11001000 /* z */
};

const unsigned char digits[] = {
	0b11111100,		// 0
	0b01111100,		// 1
	0b00111100,		// 2
	0b00011100,		// 3
	0b00001100,		// 4
	0b00000100,		// 5
	0b10000100,		// 6
	0b11000100,		// 7
	0b11100100,		// 8
	0b11110100,		// 9
};

enum ePunct {
	Punct_Period = 0,
	Punct_Comma,
	Punct_QuestionMark,
	Punct_Minus,
	Punct_LeftBracket,
	Punct_RightBracket,
};

const unsigned char punctChars[] = {
	0b01010110,		// Period
	0b11001110,		// Comma
	0b00110010,		// Question mark
	0b10000110,		// Minus sign
	0b10110100,		// Left bracket
	0b10110110,		// Right bracket
};


unsigned char getMorseChar(char c)
{
	if ((c >= 'a') && (c <= 'z')) return letters[c - 'a'];
	else if ((c >= '0') && (c <= '9')) return digits[c - '0'];

	switch (c) {
	case '.': 	return punctChars[Punct_Period];
	case ',': 	return punctChars[Punct_Comma];
	case '?': 	return punctChars[Punct_QuestionMark];
	case '-': 	return punctChars[Punct_Minus];
	case '(': 	return punctChars[Punct_LeftBracket];
	case ')': 	return punctChars[Punct_RightBracket];
	default: 	return 0;
	}
} // getMorseChar


int getCharLength(char c)
{
	int nResult = 7;
	unsigned char mask = 1;

	unsigned char chMorse = getMorseChar(c);
	if (!chMorse) return 0;
	while (!(chMorse & mask)) { mask <<= 1; --nResult; }

	return nResult;
} // getCharLength


int main()
{
	DDRB = _BV(DDB0); 			// Pin 0 = output
	PORTB |= _BV(PORTB0);		// Set pin 0 to "on" to switch the LED off.

	// Set timer 1 prescaler. This and OCR1A (see below) affect the blinking speed.
	// For possible values see Attiny45 data sheet.
	TCCR1 = _BV(CS11) | _BV(CS13);	// 1:512

	// Enable timer 1 compare match A interrupt. Could have used timer 1 overflow
	// interrupt (TOIE1) instead, but using a compare register allows a more precise
	// blinking speed setting.
	TIMSK = _BV(OCIE1A);

	// Set timer 1 output compare register A. This sets the blinking speed.
	// Lower values result in a higher frequency.
	OCR1A = 130;

	// Disable analog comparator to reduce power consumption in idle mode.
	ACSR |= _BV(ACD);

	// Globally enable interrupts
	sei();

	// Endless loop putting the MCU to sleep / idle mode as often as possible.
	// The main algorithm is executed in the timer interrupt function.
	set_sleep_mode(SLEEP_MODE_IDLE);
	for (;;) { sleep_mode(); }

	return 0;
} // main


ISR(TIMER1_COMPA_vect)
{
	if (nLenPause) {
		--nLenPause;
	}
	else if (nLenSymb) {
		--nLenSymb;
		if (!nLenSymb) {
			PORTB |= _BV(PORTB0);	// LED off
			nLenPause = nLenChar ? LENGTH_DIT : LENGTH_DAH;
		}
	}
	else if (nLenChar) {
		nLenSymb = (uMorse & 0x80) ? LENGTH_DAH : LENGTH_DIT;
		PORTB &= ~_BV(PORTB0); 		// LED on
		uMorse <<= 1;
		--nLenChar;
	}
	else if (nLenText) {
		do {
			chCur = pgm_read_byte_near(theText + nPosText++);

			if (chCur == ' ') nLenPause = PAUSE_WORD;
			else {
				nLenChar = getCharLength(chCur);
				uMorse = getMorseChar(chCur);
			}

			--nLenText;
		} while (!uMorse && nLenText);		// skip unknown characters
	}
	else {
		// Start over:
		nLenPause = PAUSE_TEXT;
		nPosText = 0;
		nLenText = strlen_P(theText);
	}

	// Reset timer 1 counter (Only necessary if timer 1 compare match interrupt instead of
	// timer 1 overflow interrupt is used)
	TCNT1 = 0;
} // ISR
