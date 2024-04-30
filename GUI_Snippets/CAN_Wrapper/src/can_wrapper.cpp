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
#include <stdio.h>
#include <string>

//============================================================================
// Public
//============================================================================

CAN_Wrapper::CAN_Wrapper(unsigned int baudrate /* = 500000 */){
	CAN_Wrapper::baudrate = baudrate;
	channelID = 0;	// Init TX Channel ID
	initDriver();
}

CAN_Wrapper::~CAN_Wrapper(){
	closePort();
	xlCloseDriver();
}

void CAN_Wrapper::setID(unsigned int id){
	txID = id;
	printf("CAN_Wrapper: TX ID is set to 0x%08X\n", txID);
}

void CAN_Wrapper::increaseChannel(){
	// Using temp variable since compiler complains about that var may be undefined
	unsigned int tempChannel = (channelID + 1) % drvConfig.channelCount;
	printf("CAN_Wrapper: Setting TX Channel to %02d, %s CM(0x%I64x)\n",
			drvConfig.channel[tempChannel].channelIndex,
			drvConfig.channel[tempChannel].name,
			drvConfig.channel[tempChannel].channelMask);
	channelID = tempChannel;
}

boolean CAN_Wrapper::txCAN(byte data[], unsigned int no_bytes){

	XLstatus status;
	XLaccess chanMaskTx = drvConfig.channel[channelID].channelMask;
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

void CAN_Wrapper::rxCANHandle(HANDLE h){
	printf("Not yet implemented\n");
}

//============================================================================
// Private
//============================================================================

void CAN_Wrapper::initDriver(){

	XLstatus status;
	unsigned int i;

	// Open the driver
	status = xlOpenDriver();

	// Get hardware configuration
	status = xlGetDriverConfig(&drvConfig);

	if (status == XL_SUCCESS) {
		_printConfig();

		channelMask = 0;

		// Check the HW for supported channels
		for (i = 0; i < drvConfig.channelCount; i++){
			// HW supports CAN
			if(drvConfig.channel[i].channelBusCapabilities & XL_BUS_ACTIVE_CAP_CAN){
				channelMask |= drvConfig.channel[i].channelMask;
			}
		}

		// Channel mask is set if support is given
		if(!channelMask){
			status = XL_ERROR;
		}

		if (DEBUGGING) printf("----------------------------------------------------------------------------------------------\n");

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
		if (DEBUGGING) printf("----------------------------------------------------------------------------------------------\n");
	}

	if (status != XL_SUCCESS)
		printf("\nCAN_Wrapper: Error during initialization of the driver! Info: %s\n", xlGetErrorString(status));
}

XLstatus CAN_Wrapper::openPort(){

	XLstatus status;

	permissionMask = channelMask;

	status = xlOpenPort(&portHandle, appName, channelMask, &permissionMask, RX_QUEUE_SIZE, XL_INTERFACE_VERSION, XL_BUS_TYPE_CAN);
	if (DEBUGGING) printf("CAN_Wrapper: Opened Port with CM=0x%I64x, PM=0x%I64x, PH=0x%02ld, Info: %s\n",
			channelMask, permissionMask, portHandle, xlGetErrorString(status));

	return status;
}

XLstatus CAN_Wrapper::closePort(){
	XLstatus status;

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

//============================================================================
// Debugging
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

