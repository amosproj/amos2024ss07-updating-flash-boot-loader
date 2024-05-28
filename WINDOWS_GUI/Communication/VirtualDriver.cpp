// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : VirtualDriver.cpp
// Author      : Michael Bauer
// Version     : 0.2
// Copyright   : MIT
// Description : Qt Virtual Driver Implementation
//============================================================================

#include <QDebug>

#include "VirtualDriver.hpp"

#include <stdint.h>

VirtualDriver::VirtualDriver() {
	this->id = 0;
	this->type = 0;
	this->own_id = 0;

}

VirtualDriver::~VirtualDriver() {
    stopRX();

    bool waitOnStop = true;
    do{
        mutex.lock();
        waitOnStop = _working;
        mutex.unlock();
    } while(waitOnStop);

    qInfo() << "Virtual Driver: Destructor finished";
}

void VirtualDriver::setID(uint32_t id){
	this->id = id;
    qInfo("Virtual Driver: TX ID is set to 0x%08X\n", this->id);
}

uint8_t VirtualDriver::initDriver(){
    qInfo("Virtual Driver init successful\n");
    emit driverInit("Virtual Driver init successful");
	return 0;
}

uint8_t VirtualDriver::txData(uint8_t *data, uint8_t no_bytes){

    QByteArray ba;
    ba.resize(no_bytes);
    for(auto i = 0; i < no_bytes; i++){
        ba[i] = data[i];
	}
    qInfo() << "Virtual Driver (TX ID="<< id << ") Data: " << ba.toStdString();
    emit txDataSentStatus("Virtual Driver TX successful");
    return 1;
}

void VirtualDriver::doRX(){
    mutex.lock();
    _working = false;
    mutex.unlock();

    qWarning("No usage RX Thread in Virtual Driver\n");
    emit rxThreadFinished();
}
