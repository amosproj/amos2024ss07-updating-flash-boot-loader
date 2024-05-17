// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : can_wrapper.hpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Header for CAN Wrapper for Vector XL-Driver Library 20.30.14
//============================================================================

#ifndef CAN_WRAPPER_HPP_
#define CAN_WRAPPER_HPP_

#if defined(_Windows) || defined(_MSC_VER) || defined (__GNUC__)
 #define  STRICT
 #include <windows.h>
#endif

#define DEBUGGING				1 		// switch for debugging prints

#define RECEIVE_EVENT_SIZE 		1 		// do not edit! Currently 1 is supported only
#define RX_QUEUE_SIZE			4096 	// internal driver queue size in CAN events
#define ERROR_INVALID_INSTANCE	-1

#define CHAN01 					0		// Index of Channel 1
#define MAX_USED_CHANNEL		1		// do not edit! Currently 1 is supported only

#include <stdio.h>

#include "can_wrapper_event.hpp"
#include "vxlapi.h"

class CAN_Wrapper {

	// Variables
	private:
        char appName[XL_MAX_APPNAME+1] 		= "AMOS fbl GUI";		// AppName, currently not registered
        XLportHandle portHandle 			= XL_INVALID_PORTHANDLE;		// Holds the port handle for communication
		XLdriverConfig drvConfig;											// Holds the driver configuration
		XLaccess channelMask 				= 0;							// Chosen channel mask
		int channelIndex					= 0;							// Chosen channel index
		XLaccess permissionMask 			= 0;							// Possible channel mask (permitted)
		unsigned int baudrate 				= 500000;						// Default baudrate

		unsigned int txID 					= 0;							// TX ID for sending CAN messages
		XLevent event;														// Template variable for TX Event

		int RXThreadRunning;												// Flag for controlling RX thread
		HANDLE RXThread;													// Handle for the RX Thread
		XLhandle msgEvent;													// Message event for SetNotification
		CAN_Wrapper_Event* clientHandle		= nullptr;						// Handle for the client to inform about


	// Methods
	public:
		CAN_Wrapper(unsigned int baudrate);
		~CAN_Wrapper();

		boolean initDriver();
		void setID(unsigned int id);

		boolean txCAN(byte data[], unsigned int no_bytes);
		HANDLE startRXThread(CAN_Wrapper_Event* h);


	private:
		XLstatus openPort();
		XLstatus closePort();

		XLstatus setBaudrate(unsigned int baudrate);
		XLstatus actChannels();
		XLstatus setNotification();

		// RX handling
		static DWORD WINAPI RXThreadHandling(LPVOID);

		// debugging methods
		void _printConfig();

};


#endif /* CAN_WRAPPER_HPP_ */
