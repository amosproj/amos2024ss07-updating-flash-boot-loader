// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : Communication.cpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Communication Layer implementation
//============================================================================

#if defined(_Windows) || defined(_MSC_VER) || defined (__GNUC__)
 #define  STRICT
 #include <windows.h>
#endif

#include <stdio.h>
#include <typeinfo>

#include "Communication.h"
#include "../UDS_Spec/uds_comm_spec.h"
#include "../Communication/VirtualDriver.h"
#include "../Communication/Can_Wrapper.hpp"


Communication::Communication() {
	curr_interface_type = 0; // Initial with Virtual Driver

	virtualDriver = VirtualDriver(); // Initialize Virtual Driver
	virtualDriver.setInterfaceID(0);

	canDriver = CAN_Wrapper(500000);
	canDriver.setInterfaceID(1);
}

Communication::~Communication() {

}

void Communication::init(uint8_t comm_interface_type){

	uint8_t init_status = 0;

	if(comm_interface_type == 0){ // Init VirtualDriver
		init_status = virtualDriver.initDriver();
	}
	else if(comm_interface_type == 1){ // Init CanDriver
		init_status = canDriver.initDriver();
		canDriver.startRXThread(dynamic_cast<CAN_Wrapper_Event*>(this));
	}

	// TODO: Error Handling
	if(!init_status){
		printf("Communication: Init successful for type %d\n", comm_interface_type);
		return;
	}
	printf("Communication: Error with init of driver with type %d\n", comm_interface_type);
	return;
}

void Communication::setCommunicationType(uint8_t comm_interface_type){

	this->curr_interface_type = comm_interface_type;
	printf("Communication: Set interface to type %d\n", comm_interface_type);
}

void Communication::setID(uint32_t id){
	if(curr_interface_type == 0){ // VirtualDriver
		virtualDriver.setID(id);
	}
	else if(curr_interface_type == 1){ // CANDriver
		canDriver.setID(id);
	}
}

void Communication::txData(uint8_t *data, uint32_t no_bytes){

	if(curr_interface_type == 0){ // Using Virtual Driver
		printf("Using Virtual Driver as TX interface\n");
		uint8_t *send_msg;
		int send_len;
		int has_next;
		int max_len_per_frame = 8; // Also use CAN Message Length
		uint32_t data_ptr = 0;
		uint8_t idx = 0;

		send_msg = starting_frame(&send_len, &has_next, max_len_per_frame, data, no_bytes, &data_ptr);
		virtualDriver.txData(send_msg, send_len);
		free(send_msg);

		if (has_next){ // Check in flow control and continue sending
			send_msg = consecutive_frame(&send_len, &has_next, max_len_per_frame, data, no_bytes, &data_ptr, &idx);
			virtualDriver.txData(send_msg, send_len);
			free(send_msg);
		}
	}

	else if(curr_interface_type == 1){ // Using CAN
		printf("Communication: Using CAN as TX interface\n");
		uint8_t *send_msg;
		int send_len;
		int has_next;
		int max_len_per_frame = 8; // CAN Message Length
		uint32_t data_ptr = 0;
		uint8_t idx = 0;

		send_msg = starting_frame(&send_len, &has_next, max_len_per_frame, data, no_bytes, &data_ptr);
		canDriver.txData(send_msg, send_len);
		free(send_msg);


		if (has_next){ // Check in flow control and continue sending
			// TODO: Wait on flow control...

			send_msg = consecutive_frame(&send_len, &has_next, max_len_per_frame, data, no_bytes, &data_ptr, &idx);
			canDriver.txData(send_msg, send_len);
			free(send_msg);
		}
	}
}

void Communication::dataReceiveHandle(){



}


void Communication::handleCANEvent(unsigned int id, unsigned short dlc, unsigned char data[]){
	printf("Was called..\n");
}
