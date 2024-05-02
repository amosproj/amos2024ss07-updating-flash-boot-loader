// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : main.cpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Main to test interaction with CAN Wrapper
//============================================================================

#if defined(_Windows) || defined(_MSC_VER) || defined (__GNUC__)
 #define  STRICT
 #include <windows.h>
#endif

#include <stdio.h>
#include "can_wrapper.hpp"

//============================================================================
// global variables
//============================================================================

int g_silent = 0;

//============================================================================
// console help
//============================================================================

void help(void){

	printf("\n----------------------------------------------------------\n");
	printf("- Keyboard commands:                                     -\n");
	printf("- 't'      Transmit a message                            -\n");
	printf("- '+'      Select transmit Id  (up)                      -\n");
	printf("- '-'      Select transmit Id  (down)                    -\n");
	printf("- 'h'      Help                                          -\n");
	printf("- 'ESC'    Exit                                          -\n");
	printf("----------------------------------------------------------\n\n");
}


//============================================================================
// main
//============================================================================

class EventHandler : public CAN_Wrapper_Event {
public:
	void handleEvent(unsigned int id, unsigned short dlc, unsigned char data[]){
		if (id == 0)
			return;

		printf("Main: Received id=%d, dlc=%d, data=", id, dlc);
		for(auto i = 0; i < dlc; i++){
			printf("%d ", data[i]);
		}
		printf("\n");
	}
};

int main() {

	int stop = 0;
	int c;
	boolean init;
	unsigned int txID = 0x01;
	byte data[]={1,2,3,4,5,6,7,8};

	CAN_Wrapper can = CAN_Wrapper(500000);
	init = can.initDriver();

	if (!init){
		Sleep(10000);
		return 0;
	}


	can.setID(txID);

	EventHandler eh = EventHandler();
	can.setRXCANHandle(&eh);
	can.startRXThread();

	// Print help after startup
	help();

	while (stop == 0){

		unsigned long n;
		INPUT_RECORD ir;

		ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &n);

		if ((n == 1) && (ir.EventType == KEY_EVENT) && (ir.Event.KeyEvent.bKeyDown)) {
			c = ir.Event.KeyEvent.uChar.AsciiChar;
			switch (c) {
				case 't': // transmit a message
					can.txCAN(data, 8);
					break;

				case '+': // Increase id
					txID++;
					can.setID(txID);
					break;

				case '-': // Decrease id
					txID--;
					can.setID(txID);
					break;


				case 'h': // show help
					help();
					break;

				case 27: // end application
					stop=1;
					break;

				default:
					break;
			}
		}
	}

	return 0;
}
