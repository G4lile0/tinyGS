/*
  QueueIdxPeeking

  LIFO / FIFO implementations can be tested by changing IMPLEMENTATION

  This example code is in the public domain.

  created 3 November 2019
  modified 04 November 2020
  by SMFSW
 */

#include <cppQueue.h>

#define	IMPLEMENTATION	FIFO
#define OVERWRITE		true

#define NB_RECS			6

typedef struct strRec {
	uint16_t	entry1;
	uint16_t	entry2;
} Rec;

Rec tab[9] = {
	{ 0x1234, 0x3456 },
	{ 0x5678, 0x7890 },
	{ 0x90AB, 0xABCD },
	{ 0xCDEF, 0xEFDC },
	{ 0xDCBA, 0xBA09 },
	{ 0x0987, 0x8765 },
	{ 0x6543, 0x2112 },
	{ 0x1234, 0x5678 },
	{ 0x7890, 0xABCD }
};

cppQueue	q(sizeof(Rec), NB_RECS, IMPLEMENTATION, OVERWRITE);	// Instantiate queue

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
}

// the loop function runs over and over again forever
void loop() {
	unsigned int i;
	
	for (i = 0 ; i < sizeof(tab)/sizeof(Rec) ; i++)
	{
		Rec rec = tab[i % (sizeof(tab)/sizeof(Rec))];
		q.push(&rec);
	}
	
	for (i = 0 ; i < NB_RECS+1 ; i++)
	{
		Rec rec = {0xffff,0xffff};
		q.peekIdx(&rec, i);
		Serial.print(" ");
		Serial.print(rec.entry1, HEX);
		Serial.print(" ");
		Serial.println(rec.entry2, HEX);
	}
	
	while(1);
}