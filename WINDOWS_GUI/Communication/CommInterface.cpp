// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : Comminterface.cpp
// Author      : Michael Bauer
// Version     : 0.2
// Copyright   : MIT
// Description : Communication Interface Implementation
//============================================================================

#include "CommInterface.hpp"

#include <QDebug>

//============================================================================
// Constructor
//============================================================================

CommInterface::CommInterface(QObject *parent): QObject(parent){
	this->id = 0;
	this->type = 0;
	this->own_id = 0;
    this->rxFilterMask = 0;

    // RX Thread is stopped by default
    this->_working =false;
    this->_abort = false;
}

CommInterface::~CommInterface(){
    this->stopRX();
}

//============================================================================
// Public Method
//============================================================================

/**
 * @brief Method to init the Driver. Need to be overwritten in inheriting class.
 * @return
 */
uint8_t CommInterface::initDriver(){
    qWarning("No usage of derived CommInterface initDriver\n");
	return 1;
}

/**
 * @brief Method to send given Data with given number of Bytes. Need to be overwritten in inheriting class.
 * @param data Given Data
 * @param no_bytes Number of Bytes of the data
 * @return
 */
uint8_t CommInterface::txData(uint8_t *data, uint8_t no_bytes){
    qWarning("No usage of derived CommInterface txData\n");
	return 0;
}

/**
 * @brief Method that is called by the startRX Thread. Here comes the RX receiving loop. Need to be overwritten in the inheriting class.
 */
void CommInterface::doRX(){
    qWarning("No usage of derived CommInterface doRX. Stopped RX Thread\n");
    emit rxThreadFinished();

    mutex.lock();
    _working = false;
    mutex.unlock();
}

//============================================================================
// Slots
//============================================================================

void CommInterface::runThread(){
    this->doRX();
}

void CommInterface::txDataSlot(const QByteArray &data){
    if(VERBOSE_COMMINTERFACE) qInfo() << "CommInterface: Slot - Received TX Data to be transmitted - Size =" << data.size() << " Bytes";

    // Unwrap the received data
    uint8_t* msg = (uint8_t*)calloc(data.size(), sizeof(uint8_t));
    if (msg != nullptr){
        for(int i= 0; i < data.size(); i++){
            msg[i] = data[i];
            //qInfo() << "Step " << i << "Data: " << msg[i];
        }
        this->txData(msg, data.size());
        free(msg);
    }
}

/**
 * @brief default implementation for setBaudrate
 * @param baudrate
 */
void CommInterface::setChannelBaudrate(unsigned int baudrate) {
    emit errorPrint("Baudrate of the selected channel cannot be changed");
}
