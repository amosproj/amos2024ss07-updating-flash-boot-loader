// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : can_wrapper.cpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : CAN Wrapper for Vector XL-Driver Library 20.30.14
//============================================================================

#include "can_wrapper.hpp"
#include <string>

//============================================================================
// Public
//============================================================================

/**
 * Constructor for CAN_Wrapper. The wrapper is initialized with a given baudrate and the port is opened for communication.
 *
 * @param unsigned int baudrate Given baudrate to be initialized.
 */
CAN_Wrapper::CAN_Wrapper(unsigned int baudrate /* = 500000 */){
	this->baudrate = baudrate;
}


/**
 * Deconstructor for CAN_Wrapper. Closes the port and driver.
 */
CAN_Wrapper::~CAN_Wrapper(){

	// Shutdown the RX thread
	RXThreadRunning = 0;

	// Close the port and the driver
	closePort();
	xlCloseDriver();
}

/**
 * Method to init the driver.
 *
 * @return boolean True if init was successful, false if there was an error
 */
boolean CAN_Wrapper::initDriver(){

	XLstatus status;
	unsigned int i;
	boolean found = false;

	// default values for registry
	unsigned int hwType = 0;
	unsigned int hwIndex = 0;
	unsigned int hwChannel = 0;
	unsigned int appChannel = 0;
	unsigned int busType = XL_BUS_TYPE_CAN;

	// Open the driver
	status = xlOpenDriver();

	// Get hardware configuration
	status = xlGetDriverConfig(&drvConfig);

	// Check if application is registered
	status = xlGetApplConfig(appName, CHAN01, &hwType, &hwIndex, &hwChannel, busType);

	if (status == XL_SUCCESS) {
		_printConfig();

		if (DEBUGGING) printf("----------------------------------------------------------------------------------------------\n");

		channelMask = 0;

		// Check application configuration for the relevant channel
		channelMask = xlGetChannelMask(hwType, hwIndex, hwChannel);

		for (i = 0; i < drvConfig.channelCount; i++){

			// Check if CAN is assigned in Vector Hardware Config
			if(	(drvConfig.channel[i].channelMask == channelMask) &&
				(drvConfig.channel[i].channelBusCapabilities & XL_BUS_ACTIVE_CAP_CAN)){
				found = true;
			}
		}

		// State Error if no assignment is given
		if(!found){
			printf("Please assign %d channel(s) in Vector Hardware Config or Vector Hardware Manager and restart the application\n", MAX_USED_CHANNEL);
			status = XL_ERROR;
		}

		// Open one port including all channels
		if (status == XL_SUCCESS){
			status = openPort();
		}

		// Set the defined BaudRate
		if ((status == XL_SUCCESS) && (portHandle != XL_INVALID_PORTHANDLE)){
			status = setBaudrate(baudrate);
		}
		else {
			xlClosePort(portHandle);
			portHandle = XL_INVALID_PORTHANDLE;
			status = XL_ERROR;
		}

		// Activate all channel on the bus
		if (status == XL_SUCCESS){
			status = actChannels();
		}

		// Get an event for every message
		if (status == XL_SUCCESS){
			status = setNotification();
		}

		if (status != XL_SUCCESS)
				printf("\nCAN_Wrapper: Error during initialization of the driver! Info: %s\n", xlGetErrorString(status));

		if (DEBUGGING) printf("----------------------------------------------------------------------------------------------\n");

	}

	else { // Application not registered yet, put some default parameters into the registry
		for(i = 0; (i < drvConfig.channelCount) && (appChannel < MAX_USED_CHANNEL); i++){
			if(drvConfig.channel[i].channelBusCapabilities & XL_BUS_ACTIVE_CAP_CAN){
				hwType = drvConfig.channel[i].hwType;
				hwIndex = drvConfig.channel[i].hwIndex;
				hwChannel = drvConfig.channel[i].hwChannel;

				status = xlSetApplConfig(	// Register the App with default settings
						appName,			// Defined Application Name
						appChannel,			// Application channel starting from 0
						hwType,				// Hardware Type
						hwIndex,			// Index of Hardware slot (0, 1, ...)
						hwChannel,			// Index of channel (=connector) (0, 1, ...)
						busType);			// Bus type need to be CAN
				appChannel++;
			}
		}
		printf("CAN_Wrapper: No HW defined\n");
		printf("\tPlease assign %d channel(s) in Vector Hardware Config or Vector Hardware Manager and restart the application\n\n", MAX_USED_CHANNEL);
	}

	return status == XL_SUCCESS;
}

/**
 * Method to set the TX ID for the transmission of CAN messages. The ID will be used directly after it is set
 *
 * @param unsigned int id: ID for the TX
 */
void CAN_Wrapper::setID(unsigned int id){
	txID = id;
	printf("CAN_Wrapper: TX ID is set to 0x%08X\n", txID);
}

/**
 * Transmits given number of bytes of the given data by using CAN
 *
 * @param byte data[] Given data (Maximal 8 byte array is possible)
 * @param unsigned int no_bytes Set the number of bytes to be transmitted (Maximum of 8 byte is possible)
 * @return boolean True if message could be transmitted
 */
boolean CAN_Wrapper::txCAN(byte data[], unsigned int no_bytes){

	XLstatus status;
	XLaccess chanMaskTx = channelMask;
	unsigned int msgCount = 1;

	// Error Handling
	if (no_bytes > 8) {
		printf("CAN_Wrapper: Maximum number of Bytes is 8");
		return false;
	}

	// Message processing

	// Reset the event variable
	memset(&event, 0, sizeof(event));

	// Fill in the data
	event.tag 				= XL_TRANSMIT_MSG;
	event.tagData.msg.id 	= txID;
	event.tagData.msg.dlc 	= no_bytes;
	event.tagData.msg.flags = 0;
	for (unsigned int i = 0; i < no_bytes; i++){
		event.tagData.msg.data[i] = data[i];
	}

	// Transmit the message
	status = xlCanTransmit(portHandle, chanMaskTx, &msgCount, &event);
	printf("CAN_Wrapper: Transmitting CAN message with CM(0x%I64x), %s\n", chanMaskTx, xlGetErrorString(status));

	return (status == XL_SUCCESS);
}

/**
 * Method to give a reference to a user defined handle that is informed about new CAN messages
 *
 * @param CAN_Wrapper_Event h: Defined handle to be used by the CAN_Wrapper
 */
void CAN_Wrapper::setRXCANHandle(CAN_Wrapper_Event* h){
	if (h != nullptr)
		clientHandle = h;
}

/**
 * Method to start a RX Thread that uses the RX CAN Handle to inform about new messages
 */
HANDLE CAN_Wrapper::startRXThread(){
	return CreateThread(0, 0, RXThreadHandling, this, 0, 0);
}

//============================================================================
// Private
//============================================================================

XLstatus CAN_Wrapper::openPort(){

	XLstatus status;

	permissionMask = channelMask;

	status = xlOpenPort(&portHandle, appName, channelMask, &permissionMask, RX_QUEUE_SIZE, XL_INTERFACE_VERSION, XL_BUS_TYPE_CAN);
	if (DEBUGGING) printf("CAN_Wrapper: Opened Port with CM=0x%I64x, PM=0x%I64x, PH=0x%02ld, Info: %s\n",
			channelMask, permissionMask, portHandle, xlGetErrorString(status));

	return status;
}

XLstatus CAN_Wrapper::closePort(){
	XLstatus status = XL_SUCCESS;

	if(portHandle != XL_INVALID_PORTHANDLE){
		status = xlClosePort(portHandle);
		if (DEBUGGING) printf("CAN_Wrapper: Closed Port with PH=0x%02ld, Info: %s\n",
							portHandle, xlGetErrorString(status));
	}
	portHandle = XL_INVALID_PORTHANDLE;
	return status;
}

XLstatus CAN_Wrapper::setBaudrate(unsigned int baudrate){

	XLstatus status;

	status = xlCanSetChannelBitrate(portHandle, channelMask, baudrate);
	if (DEBUGGING) printf("CAN_Wrapper: CanSetChannelBitrate to BaudRate=%u, Info: %s\n",
			baudrate, xlGetErrorString(status));

	return status;

}

XLstatus CAN_Wrapper::actChannels(){
	XLstatus status;

	status = xlActivateChannel(portHandle, channelMask, XL_BUS_TYPE_CAN, XL_ACTIVATE_RESET_CLOCK);
	if (DEBUGGING) printf("CAN_Wrapper: ActivateChannel CM=0x%I64x, Info: %s\n",
				channelMask, xlGetErrorString(status));

	return status;
}

XLstatus CAN_Wrapper::setNotification(){
	XLstatus status;

	status = xlSetNotification (portHandle, &msgEvent, 1);
	if (DEBUGGING) printf("CAN_Wrapper: SetNotification for every message, Info: %s\n",
				xlGetErrorString(status));

	return status;
}

//============================================================================
// Private RX handling
//============================================================================

DWORD WINAPI CAN_Wrapper::RXThreadHandling(LPVOID param){
	CAN_Wrapper *instance = static_cast<CAN_Wrapper*>(param);

	XLstatus status;

	unsigned int msgrx = RECEIVE_EVENT_SIZE;
	XLevent event;

	if (instance == nullptr)
		return ERROR_INVALID_INSTANCE;

	if (instance->clientHandle == nullptr){
		return ERROR_INVALID_ACCESS;
	}

	if ((instance->portHandle) != XL_INVALID_PORTHANDLE){
		printf("CAN_Wrapper: Starting RXThreadHandling\n");

		instance->RXThreadRunning = 1;
		while(instance->RXThreadRunning) {
			WaitForSingleObject(instance->msgEvent, 10);

			status = XL_SUCCESS;
			while(!status){

				msgrx = RECEIVE_EVENT_SIZE;

				status = xlReceive(instance->portHandle, &msgrx, &event);

				if(status != XL_ERR_QUEUE_IS_EMPTY){

					if (instance->clientHandle != nullptr){
						(*instance->clientHandle).handleEvent(event.tagData.msg.id, event.tagData.msg.dlc, event.tagData.msg.data);
					}
					else {
						printf("CAN_Wrapper: No handler defined...\n");
					}
				}
			}
		}
		return NO_ERROR;
	}

	return ERROR_CREATE_FAILED;
}

//============================================================================
// Private Debugging methods
//============================================================================

void CAN_Wrapper::_printConfig(){
	if (!DEBUGGING) return;

	printf("----------------------------------------------------------------------------------------------\n");
	printf("HW Configuration\n");
	printf("- %02d channels\n", drvConfig.channelCount);

	for(unsigned int i = 0; i < drvConfig.channelCount; i++){
		printf("\t-Ch: %02d, Mask:0x%03I64x, Name: %23s",
				drvConfig.channel[i].channelIndex,
				drvConfig.channel[i].channelMask,
				drvConfig.channel[i].name);

		if (drvConfig.channel[i].transceiverType != XL_TRANSCEIVER_TYPE_NONE)
			printf(", TX %13s", drvConfig.channel[i].transceiverName);

		printf("\n");
	}

	printf("----------------------------------------------------------------------------------------------\n");
}

