// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : UDS.cpp
// Author      : Michael Bauer
// Version     : 0.2
// Copyright   : MIT
// Description : Qt UDS Layer implementation
//============================================================================

#include <QDebug>

#include "UDS.hpp"

#include "../UDS_Spec/uds_comm_spec.h"

UDS::UDS(){
    this->init = 0;
}

UDS::UDS(uint8_t gui_id) {
	this->gui_id = gui_id;
    this->init = 1;
}

UDS::~UDS() {

}

//////////////////////////////////////////////////////////////////////////////
// Public - Receiving UDS Messages
//////////////////////////////////////////////////////////////////////////////
void UDS::messageInterpreter(UDS_Msg msg){

    qInfo() << ">> UDS: Received Msg from ID" << QString("0x%1").arg(msg.getID(), 8, 16, QLatin1Char( '0' )) << "Need to interpret. ";

    uint32_t len = 0;
    uint8_t* data = msg.getData(&len);
    qInfo() << "Length: " << len;

    for(auto i = 0; i < len; i++){
        qInfo() << data[i];
    }

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
    qInfo("<< UDS: Sending out Request for Identification to all ECUs\n");

	uint32_t id = (uint32_t)(FBLCAN_BASE_ADDRESS | this->gui_id);
	// First set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(id); // TODO: Check Architecture how to handle interface

	// Create the relevant message
	int len;
	uint8_t *msg = _create_tester_present(&len, 0, 1); // Request Tester present from ECUS
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

	// Finally transmit the data
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);
}


// Specification for Diagnostic and Communication Management
void UDS::diagnosticSessionControl(uint32_t id, uint8_t session){
    if(!init){
        return;
    }

    qInfo("<< UDS: Sending out Diagnostic Session Control\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_diagnostic_session_control(&len, 0, session);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

	// Finally transmit the data
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);
}

void UDS::ecuReset(uint32_t id, uint8_t reset_type){
    if(!init){
        return;
    }

    qInfo("<< UDS: Sending out for ECU Reset\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_ecu_reset(&len, 0, reset_type);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

	// Finally transmit the data
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);
}

void UDS::securityAccessRequestSEED(uint32_t id){
    if(!init){
        return;
    }

    qInfo("<< UDS: Sending out Security Access for Seed\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_security_access(&len, 0, FBL_SEC_ACCESS_SEED, 0, 0);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

	// Finally transmit the data
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);
}


void UDS::securityAccessVerifyKey(uint32_t id, uint8_t *key, uint8_t key_len){
    if(!init){
        return;
    }

    qInfo("<< UDS: Sending out Security Access for Verify Key\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_security_access(&len, 0, FBL_SEC_ACCESS_VERIFY_KEY, key, key_len);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

	// Finally transmit the data
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);
}


void UDS::testerPresent(uint32_t id){
    if(!init){
        return;
    }

    qInfo("<< UDS: Sending out Tester Present\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_tester_present(&len, 0, FBL_TESTER_PRES_WITHOUT_RESPONSE);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

	// Finally transmit the data
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);
}

// Specification for Data Transmission
void UDS::readDataByIdentifier(uint32_t id, uint16_t identifier){
    if(!init){
        return;
    }

    qInfo("<< UDS: Sending out Read Data By Identifier\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_read_data_by_ident(&len, 0, identifier, 0, 0);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

	// Finally transmit the data
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);
}
void UDS::readMemoryByAddress(uint32_t id, uint32_t address, uint16_t no_bytes){
    if(!init){
        return;
    }

    qInfo("<< UDS: Sending out Read Memory By Address\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_read_memory_by_address(&len, 0, address, no_bytes, 0, 0);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

	// Finally transmit the data
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);
}

void UDS::writeDataByIdentifier(uint32_t id, uint16_t identifier, uint8_t* data, uint8_t data_len){
    if(!init){
        return;
    }

    qInfo("<< UDS: Sending out Write Data By Identifier\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_write_data_by_ident(&len, 0, identifier, data, data_len);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

	// Finally transmit the data
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);
}

// Specification for Upload | Download
void UDS::requestDownload(uint32_t id, uint32_t address, uint32_t no_bytes){
    if(!init){
        return;
    }

    qInfo("<< UDS: Sending out Request Download\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_request_download(&len, 0, address, no_bytes);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

	// Finally transmit the data
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);
}

void UDS::requestUpload(uint32_t id, uint32_t address, uint32_t no_bytes){
    if(!init){
        return;
    }

    qInfo("<< UDS: Sending Request Upload \n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_request_upload(&len, 0, address, no_bytes);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

	// Finally transmit the data
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);
}

void UDS::transferData(uint32_t id, uint32_t address, uint8_t* data, uint8_t data_len){
    if(!init){
        return;
    }

    qInfo("<< UDS: Sending out Transfer Data\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_transfer_data(&len, 0xA0090000, data, data_len);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

	// Finally transmit the data
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);
}

void UDS::requestTransferExit(uint32_t id, uint32_t address){
    if(!init){
        return;
    }

    qInfo("<< UDS: Sending out Request Transfer Exit\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_request_transfer_exit(&len, 0, address);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

	// Finally transmit the data
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);
}

// Supported Common Response Codes
void UDS::negativeResponse(uint32_t id, uint8_t reg_sid, uint8_t neg_resp_code){
    if(!init){
        return;
    }

    qInfo("<< UDS: Sending out Negative Response\n");

	// First create the common ID
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);

	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

	// Create the relevant message
	int len;
	uint8_t *msg = _create_neg_response(&len, reg_sid, neg_resp_code);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

	// Finally transmit the data
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);
}

//////////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////////

uint32_t UDS::createCommonID(uint32_t base_id, uint8_t gui_id, uint32_t ecu_id){

	if (ecu_id > 0xFFF){
        qInfo("ID is not supported");
		return 0;
	}

	uint32_t send_id = (uint32_t)(base_id | gui_id);
	send_id |= (ecu_id<<4); // Combine ECU ID with send ID including GUI ID (see spec)

	return send_id;
}

//============================================================================
// Slots
//============================================================================

void UDS::rxDataReceiverSlot(const UDS_Msg &uds){
    qInfo("UDS: Slot - Received UDS Message to be processed");
    // Unwrap the data

    // TODO: Fixme - Crashes if method is called!
    //this->messageInterpreter(uds);
}
