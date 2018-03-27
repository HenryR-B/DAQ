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

/* Symbolic constants */
#define SIMULATOR	4

#define RUN			0		// Switch #0
#define RESET		1		// Switch #1

#define ON			1
#define OFF			0

#define NUM_DISPLAYS 8
#define SPACE_CHAR	0

/* timing constants*/
#define ONE_SECOND 1000
#define ONE_MILLI_SECOND 1
#define ONE_HUNDRED_MILLI_SECONDS 100
#define ONE_MINUTE 60

#define DECIMAL_MAX 10

/* Position for the digits to be written to the display */
#define DECIMAL_OFFSET 1
#define SECONDS_OFFSET 2
#define MINUTES_OFFSET 4

/* Width of the display for each type of digit */
#define DECIMAL_WIDTH 1
#define SECONDS_WIDTH 2
#define MINUTES_WIDTH 2

/* The simulator to run on */
#define SIMULATOR 4

/* Function Prototypes */
void runCounter(void);
void writeDigit(int digit, int position);
void writeNumber(int number, int length, int offset);
void checkSwitches(void);

int main(void) {

	/* Check if the DAQ has been successfully opened, if so, run the counter */
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

/*
 * Function runCounter - Runs the counter while also using other functions
 *					   - The counter counts each one tenth of a second, seconds and minutes
 *					   - Accurate to 250 Milliseconds within a time span of one minute.
 */
void runCounter(void) {
	int startCount = 0;
	int countDecimalSeconds = 0;
	int countSeconds = 0;
	int countMinutes = 0;
	int input;
	double deltaT = 0;
	int runSwitch, resetSwitch;

	checkSwitches();

	countSeconds = startCount;
	writeNumber(countSeconds, SECONDS_WIDTH, SECONDS_OFFSET);

	while (continueSuperLoop()) {
		deltaT = millis();

		/* Reads the status of both switches */
		runSwitch = digitalRead(RUN);
		resetSwitch = digitalRead(RESET);

		/* as long as the reset switch is ON, reset the count and clear the display. */
		if (resetSwitch == ON) {
			/* Make sure that the screen doesn't flicker, checking if the screen was already reset */
			if (countSeconds > startCount) {
				countSeconds = startCount;
				countDecimalSeconds = startCount;
				countMinutes = startCount;
				writeNumber(countDecimalSeconds, DECIMAL_WIDTH, DECIMAL_OFFSET);
				writeNumber(countSeconds, SECONDS_WIDTH, SECONDS_OFFSET);
				writeNumber(countMinutes, MINUTES_WIDTH, MINUTES_OFFSET);
			}
		}

		/* Otherwise, if the Run Switch is ON, run the counter and increase it*/
		else if (runSwitch == ON) {
			/* Checking that the seconds counter does not go above 60 */
			if (countSeconds < ONE_MINUTE) {
				/* Checking that the decimal seconds counter does not go above 10 */
				if (countDecimalSeconds < DECIMAL_MAX) {
					/* increase the counter */
					countDecimalSeconds++;
					/* print it to the screen */
					writeNumber(countDecimalSeconds, DECIMAL_WIDTH, DECIMAL_OFFSET);

					/* check if the user wants to quit*/
					if (!continueSuperLoop())
						return;
					/* check if the reset switch was toggled ON */
					if (digitalRead(RESET)) {
						/* if so, reset the counters and break out of the loop */
						countSeconds = startCount;
						countDecimalSeconds = startCount;
						countMinutes = startCount;
						writeNumber(countDecimalSeconds, DECIMAL_WIDTH, DECIMAL_OFFSET);
						writeNumber(countSeconds, SECONDS_WIDTH, SECONDS_OFFSET);
						writeNumber(countMinutes, MINUTES_WIDTH, MINUTES_OFFSET);
						break;
					}

					/* delay for a Maximum of one hundred milliseconds, taking into account the processing speed of the program */
					delay(ONE_HUNDRED_MILLI_SECONDS - millis() + deltaT - 15);
				}
				/* otherwise, reset the decimal counter and increase the seconds counter and write to the display */
				else {
					countDecimalSeconds = startCount;
					writeNumber(countDecimalSeconds, DECIMAL_WIDTH, DECIMAL_OFFSET);
					countSeconds++;
					writeNumber(countSeconds, SECONDS_WIDTH, SECONDS_OFFSET);

					/* delay for a Maximum of one hundred milliseconds, taking into account the processing speed of the program */
					delay(ONE_HUNDRED_MILLI_SECONDS - millis() + deltaT - 15);
				}
			}
			/* otherwise, reset the seconds counter and increase the minutes counter, and write the counts to the display */
			else {
				countSeconds = startCount;
				writeNumber(countSeconds, SECONDS_WIDTH, SECONDS_OFFSET);
				countMinutes++;
				writeNumber(countMinutes, MINUTES_WIDTH, MINUTES_OFFSET);

				/* delay for a Maximum of one hundred milliseconds, taking into account the processing speed of the program */
				delay(ONE_HUNDRED_MILLI_SECONDS - millis() + deltaT - 15);
			}
		}
	}
}

/* 
 * Function writeDigit - writes a digit to a 7-segment display at a given position
 *
 * Parameters:
 *				digit - the digit to display on the DAQ
 *				position - the index of the position where to display the digit on the DAQ
 */
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

/* 
 * Function writeNumber - writes a number to the 7-segment displays on the DAQ
 *
 * Parameters: 
 *				number - the number to write to the display
 *				length - the length of the number to display
 *				offset - the offset index, where to display the rightmost digit
 */
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

/*
 * Function checkSwitches - Checks that the switches are both off
 *						  - Loops until the user switches them off
 */
void checkSwitches(void) {
	int runSwitch, resetSwitch;

	printf("\nInitially, both switches should be off to proceed.\n");
	printf("Please make sure both switches are off.\n\n");

	do {
		runSwitch = digitalRead(RUN);
		resetSwitch = digitalRead(RESET);
	} while (runSwitch == ON || resetSwitch == ON);

	printf("Both switches are now off, proceeding ... \n");
	printf("To start/stop the counter, use the RUN switch (#0). \n");
	printf("To reset the counter, use the RESET switch (#1). \n");
}