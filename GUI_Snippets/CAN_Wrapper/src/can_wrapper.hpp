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

#include <stdio.h>

#include "lib/vxlapi.h"


class CAN_Wrapper {

	// Variables
	private:
		char appName[XL_MAX_APPNAME+1] 		= "AMOSApp";
		XLportHandle portHandle 			= XL_INVALID_PORTHANDLE;
		XLdriverConfig drvConfig;
		XLaccess channelMask 				= 0;
		XLaccess permissionMask 			= 0;
		unsigned int baudrate 				= 500000;						// Default baudrate

		unsigned int txID 					= 0;


	// Methods
	public:
		CAN_Wrapper(unsigned int baudrate);
		~CAN_Wrapper();

		void setID(unsigned int id);

		boolean txCAN(byte data[], unsigned int no_bytes);
		void rxCANHandle(HANDLE h);


	private:

		void initDriver();

		XLstatus openPort();
		XLstatus closePort();

		XLstatus setBaudrate(unsigned int baudrate);

		// debugging methods
		void _printConfig();

};


#endif /* CAN_WRAPPER_HPP_ */
