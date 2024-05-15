// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : VirtualDriver.cpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Virtual Driver Implementation
//============================================================================

#include "VirtualDriver.h"

#include <stdio.h>
#include <stdint.h>

VirtualDriver::VirtualDriver() {
	this->id = 0;
	this->type = 0;
	this->own_id = 0;

}

VirtualDriver::~VirtualDriver() {

}

void VirtualDriver::setID(uint32_t id){
	this->id = id;
	printf("Virtual Driver: TX ID is set to 0x%08X\n", this->id);
}

uint8_t VirtualDriver::initDriver(){
	printf("Virtual Driver init successful\n");
	return 0;
}

uint8_t VirtualDriver::txData(uint8_t *data, uint8_t no_bytes){
	printf("Virtual Driver (TX ID=0x%08X) - TX (HEX): ", id);
	for(auto i = 0; i < no_bytes; i++){
		printf("%02X ", data[i]);
	}
	printf("\n");
	return 1;
}
