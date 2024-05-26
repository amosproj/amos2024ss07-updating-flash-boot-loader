// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : VirtualDriver.hpp
// Author      : Michael Bauer
// Version     : 0.2
// Copyright   : MIT
// Description : Qt Virtual Driver Implementation
//============================================================================

#ifndef COMMUNICATION_VIRTUALDRIVER_H_
#define COMMUNICATION_VIRTUALDRIVER_H_

#include "CommInterface.hpp"

class VirtualDriver : public CommInterface{

public:
	VirtualDriver();
	~VirtualDriver();

	void setID(uint32_t id) override;
	uint8_t initDriver() override;

	uint8_t txData(uint8_t *data, uint8_t no_bytes) override;
    void doRX() override;
};

#endif /* COMMUNICATION_VIRTUALDRIVER_H_ */
