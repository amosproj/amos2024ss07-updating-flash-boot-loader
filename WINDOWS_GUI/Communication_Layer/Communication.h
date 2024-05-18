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

#include "../UDS_Layer/UDS_Event.h"

#include "../Communication/can_wrapper_event.hpp"
#include "../Communication/Can_Wrapper.hpp"

#include "../Communication/VirtualDriver.h"

#define COMM_INTERFACE_VIRTUAL				(0x0)
#define COMM_INTERFACE_CAN					(0x1)


class Communication : public CAN_Wrapper_Event {

private:
    UDS_Event_Handler* uds_eh;

	uint8_t curr_interface_type;

	VirtualDriver virtualDriver;
	CAN_Wrapper canDriver;

    // Used for consecutive frames
    uint32_t multiframe_curr_id;
    uint8_t *multiframe_curr_uds_msg;
    int multiframe_curr_uds_msg_len;
    uint32_t multiframe_curr_uds_msg_idx;
    int multiframe_next_msg_available;
    uint8_t multiframe_still_receiving;

public:
	Communication();
	~Communication();

    void setUDSInterpreter(UDS_Event_Handler* uds_eh);

	void init(uint8_t ct);
	void setCommunicationType(uint8_t ct);
	void setID(uint32_t id); // TODO: Check on Architecture

	void txData(uint8_t *data, uint32_t no_bytes);

    void dataReceiveHandleMulti();
	// CAN_Wrapper_Event
	void handleCANEvent(unsigned int id, unsigned short dlc, unsigned char data[]);

    // Testing
    void setTestMode();
};

#endif /* COMMUNICATION_LAYER_COMMUNICATION_H_ */
