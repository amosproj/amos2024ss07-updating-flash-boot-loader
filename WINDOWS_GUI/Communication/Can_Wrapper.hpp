// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : can_wrapper.hpp
// Author      : Michael Bauer
// Version     : 0.2
// Copyright   : MIT
// Description : Header for Qt CAN Wrapper for Vector XL-Driver Library 20.30.14
//============================================================================

#ifndef CAN_WRAPPER_HPP_
#define CAN_WRAPPER_HPP_

#if defined(_Windows) || defined(_MSC_VER) || defined (__GNUC__)
 #define  STRICT
 #include <windows.h>
#endif

#define DEBUGGING_CAN_DRIVER    0 		// switch for debugging prints
#define VERBOSE_CAN_DRIVER      0       // switch for verbose console information

#define RECEIVE_EVENT_SIZE 		1 		// do not edit! Currently 1 is supported only
#define RX_QUEUE_SIZE			4096 	// internal driver queue size in CAN events
#define ERROR_INVALID_INSTANCE	-1

#define CHAN01 					0		// Index of Channel 1
#define MAX_USED_CHANNEL		1		// do not edit! Currently 1 is supported only

#include <QByteArray>

#include "CommInterface.hpp"
#include "vxlapi.h"

class CAN_Wrapper : public CommInterface {

	// Variables
	private:
        uint8_t testMode                        = 0;                            // Used for controlling custom appname
        char testing_appNAme[XL_MAX_APPNAME+1]  = "AMOS TESTING";               // Custom Appname, mainly for testing
        char appName[XL_MAX_APPNAME+1]          = "AMOS FBL GUI";				// AppName, will be registered
        XLportHandle portHandle                 = XL_INVALID_PORTHANDLE;		// Holds the port handle for communication
        XLdriverConfig drvConfig;                                               // Holds the driver configuration
        XLaccess channelMask                    = 0;							// Chosen channel mask
        int channelIndex                        = 0;							// Chosen channel index
        XLaccess permissionMask                 = 0;							// Possible channel mask (permitted)
        unsigned int baudrate                   = 500000;						// Default baudrate

        unsigned int txID                       = 0;							// TX ID for sending CAN messages
        XLevent event;                  										// Template variable for TX Event
        XLhandle msgEvent;                                                      // Message event for SetNotification


	// Methods
	public:
		CAN_Wrapper();
		CAN_Wrapper(unsigned int baudrate);
		~CAN_Wrapper();

        void setID(uint32_t id) override;
        void setFilterMask(uint32_t mask) override;
		uint8_t initDriver() override;

        uint8_t txData(uint8_t *data, uint8_t no_bytes) override;
        void doRX() override;

        void setTestingAppname();


	private:
		XLstatus openPort();
		XLstatus closePort();

		XLstatus actChannels();
		XLstatus setNotification();
        XLstatus setBaudrate(unsigned int baudrate);

		// debugging methods
		void _printConfig();

    public slots:
        void setChannelBaudrate(unsigned int baudrate) override;
};


#endif /* CAN_WRAPPER_HPP_ */
