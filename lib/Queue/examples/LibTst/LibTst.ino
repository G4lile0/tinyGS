/*
  Lib Test

  LIFO / FIFO implementations can be tested by changing IMPLEMENTATION

  This example code is in the public domain.

  created 22 March 2017
  modified 04 November 2020
  by SMFSW
 */

#include <cppQueue.h>

#define	IMPLEMENTATION	FIFO
#define OVERWRITE		true

#define NB_PUSH			14
#define NB_PULL			11


typedef struct strRec {
	uint16_t	entry1;
	uint16_t	entry2;
} Rec;

Rec tab[6] = {
	{ 0x1234, 0x3456 },
	{ 0x5678, 0x7890 },
	{ 0x90AB, 0xABCD },
	{ 0xCDEF, 0xEFDC },
	{ 0xDCBA, 0xBA09 },
	{ 0x0987, 0x8765 }
};

cppQueue	q(sizeof(Rec), 10, IMPLEMENTATION, OVERWRITE);	// Instantiate queue

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);

	Serial.print("Queue is ");
	Serial.print(q.sizeOf());
	Serial.println(" bytes long.");
}

// the loop function runs over and over again forever
void loop() {
	unsigned int i;
	
	for (i = 0 ; i < NB_PUSH ; i++)
	{
		Rec rec = tab[i % (sizeof(tab)/sizeof(Rec))];
		q.push(&rec);
		Serial.print(rec.entry1, HEX);
		Serial.print(" ");
		Serial.print(rec.entry2, HEX);
		Serial.print(" Count ");
		Serial.print(q.getCount());
		Serial.print(" Remaining ");
		Serial.print(q.getRemainingCount());
		Serial.print(" Full? ");
		Serial.println(q.isFull());
	}
	
	Serial.print("Full?: ");
	Serial.print(q.isFull());
	Serial.print("  Nb left: ");
	Serial.println(q.getCount());
	for (i = 0 ; i < NB_PULL+1 ; i++)
	{
		Rec rec = {0xffff,0xffff};
		if (i != NB_PULL / 2)	{ Serial.print(q.pop(&rec)); }
		else					{ Serial.print("Test Peek: "); Serial.print(q.peek(&rec)); }
		Serial.print(" ");
		Serial.print(rec.entry1, HEX);
		Serial.print(" ");
		Serial.println(rec.entry2, HEX);
	}
	Serial.print("Empty?: ");
	Serial.print(q.isEmpty());
	Serial.print("  Nb left: ");
	Serial.println(q.getCount());
	
	while(1);
}