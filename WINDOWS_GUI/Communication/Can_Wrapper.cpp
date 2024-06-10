// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : can_wrapper.cpp
// Author      : Michael Bauer
// Version     : 0.2
// Copyright   : MIT
// Description : Qt CAN Wrapper for Vector XL-Driver Library 20.30.14
//============================================================================

#include <QDebug>
#include <QString>

#include "Can_Wrapper.hpp"

//============================================================================
// Public Testing
//============================================================================

void CAN_Wrapper::setTestingAppname(){
    this->testMode = 1;
}

//============================================================================
// Public
//============================================================================

CAN_Wrapper::CAN_Wrapper(){
	this->type = 1; // CAN
}

/**
 * Constructor for CAN_Wrapper. The wrapper is initialized with a given baudrate and the port is opened for communication.
 *
 * @param unsigned int baudrate Given baudrate to be initialized.
 */
CAN_Wrapper::CAN_Wrapper(unsigned int baudrate /*= 500000*/){
	this->baudrate = baudrate;
	this->type = 1; // CAN
}


/**
 * Deconstructor for CAN_Wrapper. Closes the port and driver.
 */
CAN_Wrapper::~CAN_Wrapper(){
    stopRX();

    bool waitOnStop = true;
    do{
        mutex.lock();
        waitOnStop = _working;
        mutex.unlock();
    } while(waitOnStop);

	// Close the port and the driver
	closePort();
    xlCloseDriver();

    while(portHandle != XL_INVALID_PORTHANDLE){}
    qInfo() << "CAN_Wrapper: Destructor of CAN_Wrapper finished";
}


/**
 * Method to init the driver.
 *
 * @return uint8_t 1 if init was successful, 0 if there was an error
 */
uint8_t CAN_Wrapper::initDriver(){

	XLstatus status;
	unsigned int i;
	boolean found = false;

	// default values for registry
	unsigned int hwType = 0;
	unsigned int hwIndex = 0;
	unsigned int hwChannel = 0;
	unsigned int appChannel = 0;
	unsigned int busType = XL_BUS_TYPE_CAN;

    // Check on some custom appname
    if (testMode){
        strncpy_s(appName, testing_appNAme, sizeof(appName));
        qInfo("Changed app name to %s.\n", appName);
    }

	// Open the driver
	status = xlOpenDriver();

	// Get hardware configuration
	status = xlGetDriverConfig(&drvConfig);

	// Check if application is registered
	status = xlGetApplConfig(appName, CHAN01, &hwType, &hwIndex, &hwChannel, busType);

	if (status == XL_SUCCESS) {
        emit infoPrint("CAN Driver: Opened Driver and loaded Config for Appname "+QString(appName));
		_printConfig();

        if (DEBUGGING_CAN_DRIVER) qInfo("----------------------------------------------------------------------------------------------");

		channelMask = 0;

		// Check application configuration for the relevant channel
        if(VERBOSE_CAN_DRIVER) qInfo("CAN_Wrapper: Checking Channel Mask for th relevant channel");
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
            emit errorPrint("CAN Driver: No assignment found in Configuration. Please assign the CAN Channel in Vector Hardware Config or Vector Hardware Manager and restart the application");
            qInfo() << "CAN_Wrapper: Please assign "<< MAX_USED_CHANNEL<<" CAN channel(s) in Vector Hardware Config or Vector Hardware Manager and restart the application";
			status = XL_ERROR;
		}
        else {
            if(VERBOSE_CAN_DRIVER)
                qInfo("CAN_Wrapper: Found relevant assignment");
        }

		// Open one port including all channels
        if (status == XL_SUCCESS){
            if(VERBOSE_CAN_DRIVER) qInfo("CAN_Wrapper: Opening port");
			status = openPort();
		}

		// Set the defined BaudRate

		if ((status == XL_SUCCESS) && (portHandle != XL_INVALID_PORTHANDLE)){
            if(VERBOSE_CAN_DRIVER) emit infoPrint("CAN Driver: Successfully opened Port");

            if(VERBOSE_CAN_DRIVER) qInfo("CAN_Wrapper: Set Baudrate");
            status = setBaudrate(baudrate);
		}
		else {
            emit errorPrint("CAN Driver: Could not open Port");
			xlClosePort(portHandle);
			portHandle = XL_INVALID_PORTHANDLE;
			status = XL_ERROR;
		}

		// Activate all channel on the bus
		if (status == XL_SUCCESS){
            if(VERBOSE_CAN_DRIVER) qInfo("CAN_Wrapper: Activate all channel on the bus");
            if(VERBOSE_CAN_DRIVER) emit infoPrint("CAN Driver: Successfully set the Baudrate for the opened Port");
			status = actChannels();
		}

		// Get an event for every message
		if (status == XL_SUCCESS){
            if(VERBOSE_CAN_DRIVER) emit infoPrint("CAN Driver: Successfully activated all Channel on the Bus");

            if(VERBOSE_CAN_DRIVER) qInfo("CAN_Wrapper: Get event for every message");
			status = setNotification();
		}

        if (status != XL_SUCCESS){
            emit errorPrint("CAN Driver: Error during initialization of the driver. Status Info: "+ QString(xlGetErrorString(status)));
            qInfo() << "CAN_Wrapper: Error during initialization of the driver! Info:"<<xlGetErrorString(status);

        }else{
            emit infoPrint("CAN Driver: Init successfully");
            qInfo() << "CAN_Wrapper: Initialization of the driver finished! Info:"<<xlGetErrorString(status);
        }
        if (DEBUGGING_CAN_DRIVER) qInfo("----------------------------------------------------------------------------------------------\n");

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
        emit errorPrint("CAN Driver: No assignment found in Configuration. Please assign the CAN Channel in Vector Hardware Config or Vector Hardware Manager and restart the application");
        if(VERBOSE_CAN_DRIVER) qInfo("CAN_Wrapper: No HW defined");
        qInfo() << "CAN_Wrapper: Please assign "<< MAX_USED_CHANNEL<<" CAN channel(s) in Vector Hardware Config or Vector Hardware Manager and restart the application";
        emit driverInit(xlGetErrorString(XL_ERR_INIT_ACCESS_MISSING));
        return XL_ERR_INIT_ACCESS_MISSING;
	}

    emit driverInit(xlGetErrorString(status));
	return XL_SUCCESS;
}

/**
 * Method to set the TX ID for the transmission of CAN messages. The ID will be used directly after it is set
 *
 * @param unsigned int id: ID for the TX
 */
void CAN_Wrapper::setID(uint32_t id){
	txID = id;
    if(VERBOSE_CAN_DRIVER) qInfo("CAN_Wrapper: TX ID is set to 0x%08X\n", txID);
}

/**
 * Transmits given number of bytes of the given data by using CAN
 *
 * @param byte data[] Given data (Maximal 8 byte array is possible)
 * @param unsigned int no_bytes Set the number of bytes to be transmitted (Maximum of 8 byte is possible)
 * @return boolean True if message could be transmitted
 */
uint8_t CAN_Wrapper::txData(uint8_t *data, uint8_t no_bytes) {

	XLstatus status;
	XLaccess chanMaskTx = channelMask;
	unsigned int msgCount = 1;
    QString bytes_data = "";

    if(VERBOSE_CAN_DRIVER) qInfo() << "CAN_Wrapper: txData - Sending of " << no_bytes << "bytes is requested";

	// Error Handling
	if (no_bytes > 8) {
        qInfo("CAN_Wrapper: Maximum number of Bytes is 8");
		return false;
	}
    if(VERBOSE_CAN_DRIVER) qInfo("CAN_Wrapper: Sending Signal txDataSentRequested");
    emit txDataSentRequested("CAN_Wrapper: TX requested");

	// Message processing

	// Reset the event variable
	memset(&event, 0, sizeof(event));

	// Fill in the data
	event.tag 				= XL_TRANSMIT_MSG;
	event.tagData.msg.id 	= (XL_CAN_EXT_MSG_ID | txID); // Setting Extended ID Msg
	event.tagData.msg.dlc 	= no_bytes;
	event.tagData.msg.flags = 0;
	for (unsigned int i = 0; i < no_bytes; i++){
		event.tagData.msg.data[i] = data[i];
        bytes_data.append(QString("%1").arg(uint8_t(data[i]), 2, 16, QLatin1Char( '0' )) + " ");
	}

	// Transmit the message
	status = xlCanTransmit(portHandle, chanMaskTx, &msgCount, &event);
    qInfo() << "<< CAN_Wrapper: Transmitting "<<no_bytes<<" byte CAN message (Data=" << bytes_data.trimmed().toStdString() << ") with ID (" << QString("0x%1").arg(event.tagData.msg.id, 8, 16, QLatin1Char( '0' ))<<") and CM("<<chanMaskTx<<") - Info: " <<xlGetErrorString(status);

    if(VERBOSE_CAN_DRIVER) qInfo("CAN_Wrapper: Sending Signal txDataSentStatus");
    emit txDataSentStatus(xlGetErrorString(status));
	return (status == XL_SUCCESS);
}

/**
 * @brief Sets the Baudrate for the opened channel
 * @param baudrate to be set
 * @return
 */
XLstatus CAN_Wrapper::setBaudrate(unsigned int baudrate){

	XLstatus status;

	status = xlCanSetChannelBitrate(portHandle, channelMask, baudrate);
    if (DEBUGGING_CAN_DRIVER) {
        qInfo()<<"CAN_Wrapper: CanSetChannelBitrate to BaudRate="<<baudrate<<", Info:"<< xlGetErrorString(status);
    }

    // Bugfix for "CAN-Bus not opening, when already in use by other app, e.g. CANoe"
    // Info: Baudrate can not be set twice, so the invalid access is ignored. Using the already set baudrate
    if(status == XL_ERR_INVALID_ACCESS){
        status = XL_SUCCESS;
    }

	return status;

}

//============================================================================
// Private
//============================================================================

/**
 * @brief Opens the Port
 * @return
 */
XLstatus CAN_Wrapper::openPort(){

	XLstatus status;

	permissionMask = channelMask;

	status = xlOpenPort(&portHandle, appName, channelMask, &permissionMask, RX_QUEUE_SIZE, XL_INTERFACE_VERSION, XL_BUS_TYPE_CAN);
    if (DEBUGGING_CAN_DRIVER) {
        qInfo()<<"CAN_Wrapper: Opened Port with CM=" << channelMask << ", PM="<<permissionMask<<", PH="<<portHandle<<", Info: "<<xlGetErrorString(status);
    }

	return status;
}

/**
 * @brief Closes the Port
 * @return
 */
XLstatus CAN_Wrapper::closePort(){
	XLstatus status = XL_SUCCESS;

	if(portHandle != XL_INVALID_PORTHANDLE){
		status = xlClosePort(portHandle);
        if (DEBUGGING_CAN_DRIVER) {
            qInfo()<<"CAN_Wrapper: Closed Port with PH="<<portHandle<<", Info: "<< xlGetErrorString(status);
        }
	}
	portHandle = XL_INVALID_PORTHANDLE;
	return status;
}

/**
 * @brief Activates the channel with Bus Type CAN
 * @return
 */
XLstatus CAN_Wrapper::actChannels(){
	XLstatus status;

	status = xlActivateChannel(portHandle, channelMask, XL_BUS_TYPE_CAN, XL_ACTIVATE_RESET_CLOCK);
    if (DEBUGGING_CAN_DRIVER) {
        qInfo()<<"CAN_Wrapper: ActivateChannel CM="<<channelMask<<", Info: "<<xlGetErrorString(status);
    }

	return status;
}

/**
 * @brief Sets the notification for every message
 * @return
 */
XLstatus CAN_Wrapper::setNotification(){
	XLstatus status;

	status = xlSetNotification (portHandle, &msgEvent, 1);
    if (DEBUGGING_CAN_DRIVER) {
        qInfo()<<"CAN_Wrapper: SetNotification for every message, Info: "<<xlGetErrorString(status);
    }

	return status;
}

//============================================================================
// Public RX Thread
//============================================================================

/**
 * @brief Method for RX Thread. Receiving loop of the CAN Wrapper
 */
void CAN_Wrapper::doRX(){
	XLstatus status;

	unsigned int msgrx = RECEIVE_EVENT_SIZE;
	XLevent event;

    if ((this->portHandle) != XL_INVALID_PORTHANDLE){
        qInfo("CAN_Wrapper: Starting RX\n");

        while(this->_working) {
            // Check if thread should be canceled
            mutex.lock();
            bool abort = _abort;
            mutex.unlock();

            if(abort)
                break;

            WaitForSingleObject(this->msgEvent, 10);

			status = XL_SUCCESS;
            while(!status && this->_working){

				msgrx = RECEIVE_EVENT_SIZE;
                status = xlReceive(this->portHandle, &msgrx, &event);

				if(status != XL_ERR_QUEUE_IS_EMPTY){
                    QString bytes_data = "";
                    QByteArray ba;
                    ba.resize(event.tagData.msg.dlc);
                    for(int i = 0; i < event.tagData.msg.dlc; i++){
                        ba[i] = event.tagData.msg.data[i];
                        bytes_data.append(QString("%1").arg(uint8_t(ba[i]), 2, 16, QLatin1Char( '0' )) + " ");
                    }
                    qInfo() << ">> CAN_Wrapper: Received"<<event.tagData.msg.dlc<<"byte CAN message with Data:" << bytes_data.trimmed().toStdString() << "from"<<QString("0x%1").arg(event.tagData.msg.id, 8, 16, QLatin1Char( '0' ));
                    if(VERBOSE_CAN_DRIVER) qInfo() << "CAN_Wrapper: Sending Signal rxDataReceived for ID" << QString("0x%1").arg(event.tagData.msg.id, 8, 16, QLatin1Char( '0' ));
                    unsigned int id = event.tagData.msg.id;
                    if(id & XL_CAN_EXT_MSG_ID)
                        id = id ^ XL_CAN_EXT_MSG_ID;
                    emit rxDataReceived(id, ba);
				}
			}
		}
	}
    else{
        qInfo("CAN_Wrapper: Could not start RX since Porthandle is missing. Init was not successfull");
    }

    qDebug("CAN_Wrapper: RX Thread stopped");
    emit rxThreadFinished();

    // Set _working to false, meaning the process can't be aborted anymore.
    mutex.lock();
    _working = false;
    mutex.unlock();
}

//============================================================================
// Private Debugging methods
//============================================================================

void CAN_Wrapper::_printConfig(){
    if (!DEBUGGING_CAN_DRIVER) return;

    qInfo("----------------------------------------------------------------------------------------------");
    qInfo("HW Configuration");
    qInfo() << "-"<<  drvConfig.channelCount << " channels";

	for(unsigned int i = 0; i < drvConfig.channelCount; i++){


		if (drvConfig.channel[i].transceiverType != XL_TRANSCEIVER_TYPE_NONE)
            qInfo() << "\t-Ch: " << drvConfig.channel[i].channelIndex << ", Mask: " << drvConfig.channel[i].channelMask << ", Name: " <<drvConfig.channel[i].name << "TX: " << drvConfig.channel[i].transceiverName;
        else{
            qInfo() << "\t-Ch: " << drvConfig.channel[i].channelIndex << ", Mask: " << drvConfig.channel[i].channelMask << ", Name: " <<drvConfig.channel[i].name;
        }

	}
    qInfo("----------------------------------------------------------------------------------------------\n");
}

//============================================================================
// Slots
//============================================================================
