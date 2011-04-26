/*
 * main.c
 *
 *  Created on: 24.04.2011
 *      Author: baum
 */
#include "main.h"
#include <avr/io.h>
#include <avr/interrupt.h>


#define PAUSE_CHAR 3		// The pause between each letter is 3 "dits"
#define PAUSE_WORD 5		// The pause between each word is 5 "dits"
#define PAUSE_TEXT 40		// Pause before retransmitting text.
#define LENGTH_DIT 1		// Legth of a "dit"
#define LENGTH_DAH 3		// Each "dah" equals 3 "dits" ;-)


// The text (ToDo: Put string in program space by using macros defined in avr/pgmspace.h
// (see http://www.nongnu.org/avr-libc/user-manual/pgmspace.html))

char *theText = "ein led throwie ist eine led die zusammen mit einer knopfzelle und einem magneten temporaer an objekten befestigt werden kann  die komponenten werden mit epoxidharz oder klebeband fixiert";

// Tests:
// const char *theText = "e i s h";
// const char *theText = "eish";
// const char *theText = "t m o";
// const char *theText = "tmo";


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

enum ePunct {
	Punct_Period,
	Punct_Comma,
	Punct_QuestionMark,
};

const unsigned char punctChars[] = {
	0b01010110,		// Period
	0b11001110,		// Comma
	0b00110010,		// Question mark
};

unsigned char getMorseChar(char c)
{
	if ((c >= 'a') && (c <= 'z')) return letters[c - 'a'];

	switch (c) {
	case '.':
		return punctChars[Punct_Period];
		break;

	case ',':
		return punctChars[Punct_Comma];
		break;

	case '?':
		return punctChars[Punct_QuestionMark];
		break;

	default:
		return 0;
	}
} // getMorseChar


int getCharLength(char c)
{
	int nResult = 7;
	unsigned char bit = 1;

	if ((c < 'a') || (c > 'z')) return 0;

	unsigned char chMorse = getMorseChar(c);
	while (!(chMorse & bit)) { bit <<= 1; --nResult; }

	return nResult;
} // getCharLength


int strlen(const char* str)
{
	int nResult = 0;

	while (*str++) ++nResult;
	return nResult;
} // strlen


int main()
{
	DDRB = _BV(DDB0); 			// Pin 0 = output
	PORTB |= _BV(PORTB0);		// Switch pin 0 "on" to switch the LED off.

	// Timer 1 prescaler:
	TCCR1 = _BV(CS10) | _BV(CS13);	// 1:256

	TIMSK = _BV(TOIE1); // Enable timer 1 overflow interrupt

	// Globally enable interrupts
	sei();

	// Endless loop (the whole algorithm is executed in the timer interrupt function)
	for (;;);

	return 0;
} // main


ISR(TIMER1_OVF_vect)
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
		chCur = theText[nPosText++];

		if (chCur == ' ') nLenPause = PAUSE_WORD;
		else {
			nLenChar = getCharLength(chCur);
			uMorse = getMorseChar(chCur);
		}

		--nLenText;
	}
	else {
		// Start over:
		nLenPause = PAUSE_TEXT;
		nPosText = 0;
		nLenText = strlen(theText);
	}
} // ISR
