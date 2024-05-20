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

uint8_t CommInterface::initDriver(){
    qWarning("No usage of derived CommInterface initDriver\n");
	return 1;
}

uint8_t CommInterface::txData(uint8_t *data, uint8_t no_bytes){
    qWarning("No usage of derived CommInterface txData\n");
	return 0;
}

void CommInterface::doRX(){
    mutex.lock();
    _working = false;
    mutex.unlock();

    qWarning("No usage of derived CommInterface doRX. Stopped RX Thread\n");
    emit rxThreadFinished();
}

//============================================================================
// Slots
//============================================================================

void CommInterface::runThread(){
    this->doRX();
}

void CommInterface::txDataSlot(const QByteArray &data){
    qInfo() << "CommInterface: Slot - Received TX Data to be transmitted - Size =" << data.size() << " Bytes";

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
