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


#define PAUSE_CHAR 3		// The pause between each letter is 3 "dits"
#define PAUSE_WORD 5		// The pause between each word is 5 "dits"
#define PAUSE_TEXT 40		// Pause before retransmitting text.
#define LENGTH_DIT 1		// Legth of a "dit"
#define LENGTH_DAH 3		// Each "dah" equals 3 "dits" ;-)


// The text (Resides in program space thanks to macros defined in avr/pgmspace.h
// (see http://www.nongnu.org/avr-libc/user-manual/pgmspace.html))


char theText[] PROGMEM =
"ein led-throwie ist eine led, die zusammen mit einer knopfzelle und einem "
"magneten temporaer an objekten befestigt werden kann. die komponenten werden "
"mit epoxidharz oder klebeband fixiert. led-throwies wurden vom graffiti research "
"lab (grl) als eine zerstoerungsfreie alternative zu graffiti erfunden . "
"led-throwies werden hauptsaechlich fuer streetart oder als effektmittel "
"fuer events und messen verwendet. in der streetart-szene werden throwies "
"an metallene objekte wie statuen, bruecken, gebaeude oder oeffentliche "
"transportmittel geworfen. auf messen und ausstellungen dienen led-throwies "
"als guenstige beleuchtungsmittel und eyecatcher. "
"led-throwies wurden 2006 von james powderly und evan roth im graffiti research "
"lab entwickelt. die throwies wurden wie alle entwicklungen des openlab als open "
"source-projekt unter public domain veroeffentlicht. "
"die erste vom graffiti research lab organisierte led-throwie-kampagne fand in new "
"york city statt. grl-aktivisten verteilten led-throwies an passanten und wiesen sie "
"an, diese auf die metallskulptur alamo am astor place in manhattan zu werfen. "
"eine aktion in deutschland wurde am berliner u-bahnhof kottbusser tor durchgefuehrt.";


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

	case '-':
		return punctChars[Punct_Minus];
		break;

	case '(':
		return punctChars[Punct_LeftBracket];
		break;

	case ')':
		return punctChars[Punct_RightBracket];
		break;

	default:
		return 0;
	}
} // getMorseChar


int getCharLength(char c)
{
	int nResult = 7;
	unsigned char bit = 1;

	unsigned char chMorse = getMorseChar(c);
	if (!chMorse) return 0;
	while (!(chMorse & bit)) { bit <<= 1; --nResult; }

	return nResult;
} // getCharLength


int main()
{
	DDRB = _BV(DDB0); 			// Pin 0 = output
#ifdef _DEBUG
	PORTB &= ~_BV(PORTB0);		// LED off on bread board
#else
	PORTB |= _BV(PORTB0);		// Switch pin 0 "on" to switch the LED off.
#endif

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
#ifdef _DEBUG
			PORTB &= ~_BV(PORTB0);	// LED off on bread board
#else
			PORTB |= _BV(PORTB0);	// LED off
#endif

			nLenPause = nLenChar ? LENGTH_DIT : LENGTH_DAH;
		}
	}
	else if (nLenChar) {
		nLenSymb = (uMorse & 0x80) ? LENGTH_DAH : LENGTH_DIT;
#ifdef _DEBUG
		PORTB |= _BV(PORTB0);		// LED on on bread board
#else
		PORTB &= ~_BV(PORTB0); 		// LED on
#endif
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
} // ISR