// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : Comminterface.cpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Communication Interface Implementation
//============================================================================

#include "CommInterface.h"

#include <stdio.h>

CommInterface::CommInterface(){
	this->id = 0;
	this->type = 0;
	this->own_id = 0;
}

CommInterface::~CommInterface(){}

uint8_t CommInterface::initDriver(){
	printf("No usage of derived CommInterface\n");
	return 1;
}

uint8_t CommInterface::txData(uint8_t *data, uint8_t no_bytes){
	printf("No usage of derived CommInterface\n");
	return 0;
}
