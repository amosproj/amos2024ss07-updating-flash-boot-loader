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
#include "../UDS_Layer/UDSMsg.h"


Communication::Communication() {
	curr_interface_type = 0; // Initial with Virtual Driver

	curr_id = 0; // Init receiving ID
	curr_uds_msg = NULL;
	curr_uds_msg_len = 0;
	next_msg_available = 0;
	still_receiving = 0;

	virtualDriver = VirtualDriver(); // Initialize Virtual Driver
	virtualDriver.setInterfaceID(0);

	canDriver = CAN_Wrapper(500000);
	canDriver.setInterfaceID(1);
}

Communication::~Communication() {

}

void Communication::init(uint8_t comm_interface_type){

	uint8_t init_status = 0;

	if(comm_interface_type == COMM_INTERFACE_VIRTUAL){ // Init VirtualDriver
		init_status = virtualDriver.initDriver();
	}
	else if(comm_interface_type == COMM_INTERFACE_CAN){ // Init CanDriver
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
	if(curr_interface_type == COMM_INTERFACE_VIRTUAL){ // VirtualDriver
		virtualDriver.setID(id);
	}
	else if(curr_interface_type == COMM_INTERFACE_CAN){ // CANDriver
		canDriver.setID(id);
	}
}

void Communication::txData(uint8_t *data, uint32_t no_bytes){

	if(curr_interface_type == COMM_INTERFACE_VIRTUAL){ // Using Virtual Driver
		printf("Using Virtual Driver as TX interface\n");
		uint8_t *send_msg;
		int send_len;
		int has_next;
		int max_len_per_frame = 8; // Also use CAN Message Length
		uint32_t data_ptr = 0;
		uint8_t idx = 0;

		send_msg = tx_starting_frame(&send_len, &has_next, max_len_per_frame, data, no_bytes, &data_ptr);
		virtualDriver.txData(send_msg, send_len);
		free(send_msg);

		if (has_next){ // Check in flow control and continue sending
			// TODO: Wait on flow control...

			while(has_next){
				send_msg = tx_consecutive_frame(&send_len, &has_next, max_len_per_frame, data, no_bytes, &data_ptr, &idx);
				virtualDriver.txData(send_msg, send_len);
				free(send_msg);
			}
		}
	}

	else if(curr_interface_type == COMM_INTERFACE_CAN){ // Using CAN
		printf("Communication: Using CAN as TX interface\n");
		uint8_t *send_msg;
		int send_len;
		int has_next;
		int max_len_per_frame = 8; // CAN Message Length
		uint32_t data_ptr = 0;
		uint8_t idx = 0;

		send_msg = tx_starting_frame(&send_len, &has_next, max_len_per_frame, data, no_bytes, &data_ptr);
		canDriver.txData(send_msg, send_len);
		free(send_msg);


		if (has_next){ // Check in flow control and continue sending
			// TODO: Wait on flow control...

			while(has_next){
				send_msg = tx_consecutive_frame(&send_len, &has_next, max_len_per_frame, data, no_bytes, &data_ptr, &idx);
				canDriver.txData(send_msg, send_len);
				free(send_msg);
			}
		}
	}
}

void Communication::dataReceiveHandle(){

	if(still_receiving == 1 && next_msg_available == 0 && curr_uds_msg != NULL){
		// Message fully received
		printf(">> Communication UDS: Received id=0x%08X, len=%d, data=", curr_id, curr_uds_msg_len);
		for(auto i = 0; i < curr_uds_msg_len; i++){
			printf("0x%02X ", curr_uds_msg[i]);
		}
		printf("\n");

		// TODO: Create UDS Message

		// Reset both receiving flags and ID
		still_receiving = 0;
		curr_id = 0;

		curr_uds_msg_len = 0;
		curr_uds_msg = NULL;
	}
}


void Communication::handleCANEvent(unsigned int id, unsigned short dlc, unsigned char data[]){

	// Debugging
	/*
	printf(">> Communication handleCANEvent: Received id=0x%08X, len=%d, data=", id, dlc);
	for(auto i = 0; i < dlc; i++){
		printf("0x%02X ", data[i]);
	}
	printf("\n");
	*/

	// Real processing

	if(curr_interface_type != COMM_INTERFACE_CAN) // CAN is not allowed to forward messages
		return;

	if(curr_id != 0 && id != curr_id){ // Ignore other IDs
		printf("Communication: Ignoring 0x%08X. Still processing communication with 0x%08X", id, curr_id);
		return;
	}

	uint8_t starting_frame = rx_is_starting_frame(data, dlc, MAX_FRAME_LEN_CAN);
	if(starting_frame){
		still_receiving = 1;
		curr_id = id;
		curr_uds_msg_idx = 0;
		curr_uds_msg = rx_starting_frame(&curr_uds_msg_len, &next_msg_available, MAX_FRAME_LEN_CAN, data, dlc);

		if(next_msg_available)
			curr_uds_msg_idx = 6; // First 6 bytes contained in First Frame
	}

	uint8_t consecutive_frame = rx_is_consecutive_frame(data, dlc, MAX_FRAME_LEN_CAN);
	if(consecutive_frame){
		still_receiving = 1;
		rx_consecutive_frame(&curr_uds_msg_len, curr_uds_msg, &next_msg_available, dlc, data, &curr_uds_msg_idx);
	}

	this->dataReceiveHandle();
}
