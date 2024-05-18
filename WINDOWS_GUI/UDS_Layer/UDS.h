// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : UDS.h
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : UDS Layer implementation
//============================================================================
#ifndef UDS_LAYER_UDS_H_
#define UDS_LAYER_UDS_H_

#include "stdint.h"

#include "../Communication_Layer/Communication.h"
#include "UDS_Event.h"
#include "../GUI_Events/consoleUpdater.h"

class UDS : public UDS_Event_Handler{

private:
	uint8_t gui_id;
	Communication *comm;
    uint8_t init;
    ConsoleUpdater* console;


public:
    UDS();
    UDS(uint8_t gui_id, Communication *comm_connection);
	virtual ~UDS();

    void setGUIConsoleConnection(ConsoleUpdater* console);
    void messageInterpreter(UDS_Msg msg);

	void reqIdentification(); // Sending out broadcast for tester present

	// Specification for Diagnostic and Communication Management
	void diagnosticSessionControl(uint32_t id, uint8_t session);
	void ecuReset(uint32_t id, uint8_t session);
	void securityAccessRequestSEED(uint32_t id);
	void securityAccessVerifyKey(uint32_t id, uint8_t *key, uint8_t key_len);
	void testerPresent(uint32_t id);

	// Specification for Data Transmission
	void readDataByIdentifier(uint32_t id, uint16_t identifier);
	void readMemoryByAddress(uint32_t id, uint32_t address, uint16_t no_bytes); // TODO: Check on Architecture
	void writeDataByIdentifier(uint32_t id, uint16_t identifier, uint8_t* data, uint8_t data_len);

	// Specification for Upload | Download
	void requestDownload(uint32_t id, uint32_t address, uint32_t no_bytes); // TODO: Check on Architecture
	void requestUpload(uint32_t id, uint32_t address, uint32_t no_bytes); // TODO: Check on Architecture
	void transferData(uint32_t id, uint32_t address, uint8_t* data, uint8_t data_len); // TODO: Check on Architecture
	void requestTransferExit(uint32_t id, uint32_t address);

	// Supported Common Response Codes
	void negativeResponse(uint32_t id, uint8_t reg_sid, uint8_t neg_resp_code);


private:
	uint32_t createCommonID(uint32_t base_id, uint8_t gui_id, uint32_t ecu_id);
};

#endif /* UDS_LAYER_UDS_H_ */
