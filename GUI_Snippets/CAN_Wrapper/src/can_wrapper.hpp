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

#include <stdio.h>

#include "can_wrapper_event.hpp"
#include "lib/vxlapi.h"

class CAN_Wrapper {

	// Variables
	private:
		char appName[XL_MAX_APPNAME+1] 		= "AMOSApp";					// AppName, currently not registered
		XLportHandle portHandle 			= XL_INVALID_PORTHANDLE;		// Holds the port handle for communication
		XLdriverConfig drvConfig;											// Holds the driver configuration
		XLaccess channelMask 				= 0;							// Chosen channel mask
		XLaccess permissionMask 			= 0;							// Possible channel mask (permitted)
		unsigned int baudrate 				= 500000;						// Default baudrate

		unsigned int txID 					= 0;							// TX ID for sending CAN messages
		unsigned int channelID				= 0;							// Used channel for TX
		XLevent event;														// Template variable for TX Event

		int RXThreadRunning;												// Flag for controlling RX thread
		HANDLE RXThread;													// Handle for the RX Thread
		XLhandle msgEvent;													// Message event for SetNotification
		CAN_Wrapper_Event* clientHandle		= nullptr;						// Handle for the client to inform about


	// Methods
	public:
		CAN_Wrapper(unsigned int baudrate);
		~CAN_Wrapper();

		void setID(unsigned int id);
		void increaseChannel();

		boolean txCAN(byte data[], unsigned int no_bytes);
		void setRXCANHandle(CAN_Wrapper_Event* h);
		HANDLE startRXThread();


	private:

		void initDriver();

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
