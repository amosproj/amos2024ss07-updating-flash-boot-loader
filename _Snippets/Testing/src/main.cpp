// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : main.cpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Sample Implementation
//============================================================================

#if defined(_Windows) || defined(_MSC_VER) || defined (__GNUC__)
 #define  STRICT
 #include <windows.h>
#endif

#include <stdio.h>
#include <list>

#include "UDS_Layer/UDS.h"
#include "Communication_Layer/Communication.h"
#include "Communication_Layer/CommInterface.h"
#include "Communication/Can_Wrapper.hpp"


int main() {

	printf("Main: Create Communication Layer\n");
	Communication comm = Communication();
	comm.setCommunicationType(1); // Set to CAN
	comm.init(1); // Set to CAN

	uint32_t ecu_id = 0x001;

	printf("Main: Create UDS Layer\n");
	UDS uds = UDS(0x001, &comm);

	printf("Main: Sending out some UDS Messages\n");
	uds.diagnosticSessionControl(ecu_id, 0x01);

	while(1){};
	return 0;
}
