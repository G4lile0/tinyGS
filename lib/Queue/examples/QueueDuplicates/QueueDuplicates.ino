/*
  QueueDuplicates

  q_PeekIdx implementation can be tested instead of q_peekPrevious by commenting USE_PEEK_PREVIOUS

  This example code is in the public domain.

  created 3 November 2019
  modified 04 November 2020
  by SMFSW
 */

#include <cppQueue.h>

#define	OVERWRITE		true

#define	USE_PEEK_PREVIOUS	// Comment this line to check the whole queue instead of just previous record

#define NB_RECS			7

typedef struct strRec {
	uint16_t	entry1;
	uint16_t	entry2;
} Rec;

Rec tab[14] = {
	{ 0x1234, 0x3456 },
	{ 0x1234, 0x3456 },
	{ 0x5678, 0x7890 },
	{ 0x5678, 0x7890 },
	{ 0x90AB, 0xABCD },
	{ 0x90AB, 0xABCD },
	{ 0xCDEF, 0xEFDC },
	{ 0xCDEF, 0xEFDC },
	{ 0xDCBA, 0xBA09 },
	{ 0xDCBA, 0xBA09 },
	{ 0x0987, 0x8765 },
	{ 0x0987, 0x8765 },
	{ 0x6543, 0x2112 },
	{ 0x6543, 0x2112 }
};

cppQueue	q(sizeof(Rec), NB_RECS, FIFO, OVERWRITE);	// Instantiate queue

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

#ifdef USE_PEEK_PREVIOUS	// Check only previous record
		Rec chk = {0xffff,0xffff};
		q.peekPrevious(&chk);

		if (memcmp(&rec, &chk, sizeof(Rec)))	{ q.push(&rec); }
#else						// Check the whole queue
		bool duplicate = false;
		for (int j = (int) q_getCount(&q) - 1 ; j >= 0 ; j--)
		{
			Rec chk = {0xffff,0xffff};
			q.peekIdx(&chk, j);
			
			if (!memcmp(&rec, &chk, sizeof(Rec)))
			{
				duplicate = true;
				break;
			}
		}
		
		if (!duplicate)	{ q.push(&rec); }
#endif
	}
	
	for (i = 0 ; i < NB_RECS ; i++)
	{
		Rec rec = {0xffff,0xffff};
		q.pop(&rec);
		Serial.print(" ");
		Serial.print(rec.entry1, HEX);
		Serial.print(" ");
		Serial.println(rec.entry2, HEX);
	}
	
	while(1);
}