/*
  RolloverTest
  Simple test to test queue rollover (for lib testing purposes mainly)
  
  Queues an incrementing counter & pop a record from queue each cycle

  This example code is in the public domain.

  created 22 March 2017
  modified 04 November 2020
  by SMFSW
 */

#include <cppQueue.h>

uint16_t in = 0;

cppQueue	q(sizeof(in), 10, FIFO);	// Instantiate queue

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	
	q.push(&in);		// First push into queue
}

// the loop function runs over and over again forever
void loop() {
	uint16_t out;
	
	q.push(&(++in));
	q.pop(&out);
	Serial.println(out);
	delay(200);
}
