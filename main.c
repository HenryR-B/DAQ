/*
* Author: Henry Bryant
* Date:     3/22/2018
*
* Purpose:  Runs a counter controlled by the user, using other functions.
*
*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <DAQlib.h>
#include <Windows.h>

#define SIMULATOR	4

#define RUN			0		// Switch #0
#define RESET		1		// Switch #1

#define TRUE		1
#define FALSE		0

#define ON			1
#define OFF			0

#define NUM_DISPLAYS 8
#define SPACE_CHAR	0

#define ONE_SECOND 1000
#define ONE_MILLI_SECOND 1
#define ONE_MINUTE 60

#define DECIMAL 2

#define ONE_HUNDRED_MILLI_SECONDS 100

#define SIMULATOR 4

#define ZEROS 00000000

void runCounter(void);
void writeDigit(int digit, int position);
void writeNumber(int number, int length, int offset);

int main(void) {

	if (setupDAQ(SIMULATOR)) {
		printf("Counter is starting...\n\n");
		runCounter();
		printf("\nThe Counter has finished.\n\n");
	}
	else
		printf("Error Opening DAQ...\n\n");


	system("PAUSE");
	return 0;
}

void runCounter(void) {
	int startCount;
	int countDecimalSeconds = 0;
	int countSeconds = 0;
	int countMinutes = 0;
	int countHours = 0;
	int runSwitch, resetSwitch;
	int input;
	double time = 0;
	double deltaT = 0;
	double deltaTLoop = 0;
	double extraTime = 0;
	double extraTime2 = 0;
	double time3;
	double val, val2;

	printf("\nInitially, both switches should be off to proceed.\n");
	printf("Please make sure both switches are off.\n\n");

	do {
		runSwitch = digitalRead(RUN);
		resetSwitch = digitalRead(RESET);
	} while (runSwitch == TRUE || resetSwitch == TRUE);

	printf("Both switches are now off, proceeding ... \n");
	printf("To start/stop the counter, use the RUN switch (#0). \n");
	printf("To reset the counter, use the RESET switch (#1). \n");

	startCount = 0;
	countSeconds = startCount;
	writeNumber(countSeconds, 4, 2);

	while (continueSuperLoop()) {
		extraTime = millis();

		runSwitch = digitalRead(RUN);
		resetSwitch = digitalRead(RESET);

		if (resetSwitch == ON) {
			if (countSeconds > startCount) {
				countSeconds = startCount;
				countDecimalSeconds = startCount;
				countMinutes = startCount;
				writeNumber(countDecimalSeconds, 1, 1);
				writeNumber(countSeconds, 2, 2);
				writeNumber(countMinutes, 2, 4);
			}
		}
		else if (runSwitch == ON) {
			if (countSeconds < ONE_MINUTE) {
				if (countDecimalSeconds < 10) {
					time = millis();
					extraTime2 = millis();
					countDecimalSeconds++;
					writeNumber(countDecimalSeconds, 1, 1);

					if (!continueSuperLoop())
						return;
					if (digitalRead(RESET)) {
						countSeconds = startCount;
						countDecimalSeconds = startCount;
						countMinutes = startCount;
						writeNumber(countDecimalSeconds, 1, 1);
						writeNumber(countSeconds, 2, 2);
						writeNumber(countMinutes, 2, 4);
						break;
					}
					delay(ONE_HUNDRED_MILLI_SECONDS - (millis() - time) - (extraTime2 - extraTime) - 15);
				}
				else {
					countDecimalSeconds = startCount;
					writeNumber(countDecimalSeconds, 1, 1);
					countSeconds++;
					writeNumber(countSeconds, 2, 2);
					delay(ONE_HUNDRED_MILLI_SECONDS - (millis() - time) - (extraTime2 - extraTime) - 15);
				}
			}
			else {
				countSeconds = startCount;
				writeNumber(countSeconds, 2, 2);
				countMinutes++;
				writeNumber(countMinutes, 2, 4);
				delay(ONE_HUNDRED_MILLI_SECONDS - (millis() - time) - (extraTime2 - extraTime) - 15);
			}
		}
	}
}

/* writes a digit to a 7-segment display at a given position */
void writeDigit(int digit, int position) {
	/*
	* const variables cannot be changed during program execution
	*   i.e. they are constants
	*/
	const int DIGITS_TABLE[10] = {
		252, 96, 218, 242, 102, 182, 190, 224, 254, 246 };

	/* only try to write a valid digit */
	if (digit >= 0 && digit <= 9) {
		displayWrite(DIGITS_TABLE[digit], position);
	}
}

/* writes a number to the 7-segment displays on the DAQ */
void writeNumber(int number, int length, int offset) {
	/* next digit */
	int digit = 0;

	/* start at the right-most 7-segment display */
	int pos = 0;

	if (length == 0)
		return;

	/* extract and write digits one at a time */
	do {
		/* get next digit */
		digit = number % 10;
		number = number / 10;

		/* write the digit to the 7 segment-display */
		writeDigit(digit, pos + offset);

		/* move to next display position */
		pos++;

		/* continue while still digits and displays remaining */
	} while (pos < length && pos < NUM_DISPLAYS);

	/* turn off any displays not used above */
	while (pos < NUM_DISPLAYS && pos < length) {
		displayWrite(SPACE_CHAR, pos);
		pos++;
	}
}