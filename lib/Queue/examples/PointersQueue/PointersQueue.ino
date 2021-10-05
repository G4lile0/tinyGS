/*
  Pointers Queue
  Pointers queue demonstration

  This example code is in the public domain.

  created 25 May 2018
  modified 04 November 2020
  by SMFSW
 */

#include <cppQueue.h>


const char * str[3] = {
	">>> This example demonstrates how to strip quotes",
	">>> from strings using a queue of pointers",
	">>> to access methods from the string class."
};

cppQueue	q(sizeof(String *), 3, FIFO);	// Instantiate queue


// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
}

// the loop function runs over and over again forever
void loop() {
	String strings[3];
	String * pStr;
	
	Serial.println("Original text:");
	for (unsigned int i = 0 ; i < 3 ; i++)
	{
		strings[i] = str[i];
		pStr = &strings[i];
		q.push(&pStr);
		Serial.println(strings[i]);
	}
	
	Serial.println("");
	Serial.println("Processed text:");
	for (unsigned int i = 0 ; i < 3 ; i++)
	{
		q.pop(&pStr);
		pStr->remove(0, 4);
		Serial.println(*pStr);
	}
	
	while(1);
}
