// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : UDSMsg.h
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : UDS Message Class
//============================================================================

#ifndef UDS_LAYER_UDSMSG_H_
#define UDS_LAYER_UDSMSG_H_

#include "stdint.h"

class UDS_Msg {

private:
    uint32_t id;
	uint8_t *data;
	uint32_t len;

public:
    UDS_Msg(uint32_t id, uint8_t *data, uint32_t len);
	virtual ~UDS_Msg();

	uint8_t getSID();
	uint8_t* getData(uint32_t *len);
    uint32_t getID();
};

#endif /* UDS_LAYER_UDSMSG_H_ */
