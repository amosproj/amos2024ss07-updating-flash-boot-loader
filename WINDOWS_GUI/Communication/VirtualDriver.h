// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : VirtualDriver.h
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Virtual Driver Implementation
//============================================================================

#ifndef COMMUNICATION_VIRTUALDRIVER_H_
#define COMMUNICATION_VIRTUALDRIVER_H_

#include "../Communication_Layer/CommInterface.h"

class VirtualDriver : public CommInterface{

public:
	VirtualDriver();
	~VirtualDriver();

	void setID(uint32_t id) override;

	uint8_t initDriver() override;
	uint8_t txData(uint8_t *data, uint8_t no_bytes) override;
};

#endif /* COMMUNICATION_VIRTUALDRIVER_H_ */
