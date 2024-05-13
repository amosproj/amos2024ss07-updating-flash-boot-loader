// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : can_wrapper_event.hpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Header for Message Event Handling (RX) for the CAN Wrapper
//============================================================================

#ifndef CAN_WRAPPER_EVENT_HPP_
#define CAN_WRAPPER_EVENT_HPP_

class CAN_Wrapper_Event {

public:
	virtual ~CAN_Wrapper_Event() = default;
	virtual void handleEvent(unsigned int id, unsigned short dlc, unsigned char data[]) = 0;
};

#endif /* CAN_WRAPPER_EVENT_HPP_ */
