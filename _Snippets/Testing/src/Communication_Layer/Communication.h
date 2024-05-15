// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : Communication.h
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Communication Layer implementation
//============================================================================

#ifndef COMMUNICATION_LAYER_COMMUNICATION_H_
#define COMMUNICATION_LAYER_COMMUNICATION_H_

#include <stdint.h>
#include <list>

#include "../Communication/can_wrapper_event.hpp"
#include "../Communication/Can_Wrapper.hpp"

#include "../Communication/VirtualDriver.h"

class Communication : public CAN_Wrapper_Event {

private:
	uint8_t curr_interface_type;

	VirtualDriver virtualDriver;
	CAN_Wrapper canDriver;

public:
	Communication();
	~Communication();

	void init(uint8_t ct);
	void setCommunicationType(uint8_t ct);
	//void setID(uint32_t id, CommInterface ci);
	void setID(uint32_t id); // TODO: Check on Architecture

	void txData(uint8_t *data, uint32_t no_bytes);
	void dataReceiveHandle();

	// CAN_Wrapper_Event
	void handleCANEvent(unsigned int id, unsigned short dlc, unsigned char data[]);
};

#endif /* COMMUNICATION_LAYER_COMMUNICATION_H_ */
