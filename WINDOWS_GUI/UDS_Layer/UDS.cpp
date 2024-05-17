// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : UDS.cpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : UDS Layer implementation
//============================================================================

#include "UDS.h"

#include <stdio.h>

#include "../UDS_Spec/uds_comm_spec.h"

UDS::UDS(){
    this->init = 0;
}

UDS::UDS(uint8_t gui_id, Communication *comm_connection) {
	this->gui_id = gui_id;
	this->comm = comm_connection;
    this->init = 1;
}

UDS::~UDS() {

}

//////////////////////////////////////////////////////////////////////////////
// Public - Receiving UDS Messages
//////////////////////////////////////////////////////////////////////////////
void UDS::messageInterpreter(UDS_Msg msg){

    printf(">> UDS: Received Msg from 0x%08X. Need to interpret: ", msg.getID());
    uint32_t len = 0;
    uint8_t* data = msg.getData(&len);
    for(auto i = 0; i < len; i++){
        printf("0x%02X ", data[i]);
    }
    printf("\n");

    // TODO: Implement UDS Message Interpreter
}


//////////////////////////////////////////////////////////////////////////////
// Public - Sending TX UDS Messages
//////////////////////////////////////////////////////////////////////////////


void UDS::reqIdentification() // Sending out broadcast for tester present
{
    if(!init){
        return;
    }
	printf("UDS: Sending out Request for Identification to all ECUs\n");

	uint32_t id = (uint32_t)(FBLCAN_BASE_ADDRESS | this->gui_id);
	// First set the right ID to be used for transmitting
	(*comm).setID(id); // TODO: Check Architecture how to handle interface

	// Create the relevant message
	int len;
	uint8_t *msg = _create_tester_present(&len, 0, 1); // Request Tester present from ECUS

	// Finally transmit the data
	(*comm).txData(msg, len);
}


// Specification for Diagnostic and Communication Management
void UDS::diagnosticSessionControl(uint32_t id, uint8_t session){
    if(!init){
        return;
    }

	printf("UDS: Sending out Diagnostic Session Control\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
	(*comm).setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_diagnostic_session_control(&len, 0, session);

	// Finally transmit the data
	(*comm).txData(msg, len);
}

void UDS::ecuReset(uint32_t id, uint8_t reset_type){
    if(!init){
        return;
    }

	printf("UDS: Sending out for ECU Reset\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
	(*comm).setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_ecu_reset(&len, 0, reset_type);

	// Finally transmit the data
	(*comm).txData(msg, len);
}

void UDS::securityAccessRequestSEED(uint32_t id){
    if(!init){
        return;
    }

	printf("UDS: Sending out Security Access for Seed\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
	(*comm).setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_security_access(&len, 0, FBL_SEC_ACCESS_SEED, 0, 0);

	// Finally transmit the data
	(*comm).txData(msg, len);
}


void UDS::securityAccessVerifyKey(uint32_t id, uint8_t *key, uint8_t key_len){
    if(!init){
        return;
    }

	printf("UDS: Sending out Security Access for Verify Key\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
	(*comm).setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_security_access(&len, 0, FBL_SEC_ACCESS_VERIFY_KEY, key, key_len);

	// Finally transmit the data
	(*comm).txData(msg, len);
}


void UDS::testerPresent(uint32_t id){
    if(!init){
        return;
    }

    printf("UDS: Sending out Test Present\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
	(*comm).setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_tester_present(&len, 0, FBL_TESTER_PRES_WITHOUT_RESPONSE);

	// Finally transmit the data
	(*comm).txData(msg, len);
}

// Specification for Data Transmission
void UDS::readDataByIdentifier(uint32_t id, uint16_t identifier){
    if(!init){
        return;
    }

    printf("UDS: Sending out Read Data By Identifier\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
	(*comm).setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_read_data_by_ident(&len, 0, identifier, 0, 0);

	// Finally transmit the data
	(*comm).txData(msg, len);
}
void UDS::readMemoryByAddress(uint32_t id, uint32_t address, uint16_t no_bytes){
    if(!init){
        return;
    }

    printf("UDS: Sending out Read Memory By Address\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
	(*comm).setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_read_memory_by_address(&len, 0, address, no_bytes, 0, 0);

	// Finally transmit the data
	(*comm).txData(msg, len);
}

void UDS::writeDataByIdentifier(uint32_t id, uint16_t identifier, uint8_t* data, uint8_t data_len){
    if(!init){
        return;
    }

    printf("UDS: Sending out Write Data By Identifier\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
	(*comm).setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_write_data_by_ident(&len, 0, identifier, data, data_len);

	// Finally transmit the data
	(*comm).txData(msg, len);
}

// Specification for Upload | Download
void UDS::requestDownload(uint32_t id, uint32_t address, uint32_t no_bytes){
    if(!init){
        return;
    }

    printf("UDS: Sending out Request Download\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
	(*comm).setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_request_download(&len, 0, address, no_bytes);

	// Finally transmit the data
	(*comm).txData(msg, len);
}

void UDS::requestUpload(uint32_t id, uint32_t address, uint32_t no_bytes){
    if(!init){
        return;
    }

    printf("UDS: Sending Request Upload \n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
	(*comm).setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_request_upload(&len, 0, address, no_bytes);

	// Finally transmit the data
	(*comm).txData(msg, len);
}

void UDS::transferData(uint32_t id, uint32_t address, uint8_t* data, uint8_t data_len){
    if(!init){
        return;
    }

    printf("UDS: Sending out Transfer Data\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
	(*comm).setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_transfer_data(&len, 0xA0090000, data, data_len);

	// Finally transmit the data
	(*comm).txData(msg, len);
}

void UDS::requestTransferExit(uint32_t id, uint32_t address){
    if(!init){
        return;
    }

    printf("UDS: Sending out Request Transfer Exit\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
	(*comm).setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_request_transfer_exit(&len, 0, address);

	// Finally transmit the data
	(*comm).txData(msg, len);
}

// Supported Common Response Codes
void UDS::negativeResponse(uint32_t id, uint8_t reg_sid, uint8_t neg_resp_code){
    if(!init){
        return;
    }

    printf("UDS: Sending out Negative Response\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
	(*comm).setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_neg_response(&len, reg_sid, neg_resp_code);

	// Finally transmit the data
	(*comm).txData(msg, len);
}

//////////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////////

uint32_t UDS::createCommonID(uint32_t base_id, uint8_t gui_id, uint32_t ecu_id){

	if (ecu_id > 0xFFF){
		printf("ID is not supported");
		return 0;
	}

	uint32_t send_id = (uint32_t)(base_id | gui_id);
	send_id |= (ecu_id<<4); // Combine ECU ID with send ID including GUI ID (see spec)

	return send_id;
}
