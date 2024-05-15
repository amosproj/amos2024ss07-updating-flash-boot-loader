// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : Comminterface.h
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Communication Interface Implementation
//============================================================================

#ifndef COMMUNICATION_LAYER_COMMINTERFACE_H_
#define COMMUNICATION_LAYER_COMMINTERFACE_H_

#include <stdint.h>

class CommInterface {

protected:
	uint8_t type; // 0 = Virtual Driver, 1 = CAN, 2 = CAN_FD
	uint8_t own_id; // ID to identify the interface

	uint32_t id; // ID for transmitting

public:
	CommInterface();
	virtual ~CommInterface();

	uint8_t getType(){
		return this->type;
	}

	uint8_t getInterfaceID(){
		return this->own_id;
	}

	void setInterfaceID(uint8_t id){
		this->own_id = id;
	}

	virtual void setID(uint32_t id){
		this->id = id;
	}

	virtual uint8_t initDriver();
	virtual uint8_t txData(uint8_t *data, uint8_t no_bytes);

};

#endif /* COMMUNICATION_LAYER_COMMINTERFACE_H_ */
