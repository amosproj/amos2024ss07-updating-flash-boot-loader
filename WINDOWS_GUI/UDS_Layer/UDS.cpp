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
void UDS::messageInterpreter(unsigned int id, uint8_t *data, uint8_t no_bytes){

    QString s;
    QTextStream out(&s);

    QString console = ">> UDS Received from ID: ";
    console.append(QString("0x%1").arg(id, 8, 16, QLatin1Char( '0' )));
    console.append(" - Data=");

    for(auto i = 0; i < no_bytes; i++){
        console.append(" " + QString("%1").arg(uint8_t(data[i]), 2, 16, QLatin1Char( '0' )));
    }
    qInfo() << console.toStdString();
    //emit toConsole(console);

    if(no_bytes == 0) {
        out << "UDS: No data passed";
        return;
    }
    uint8_t SID = data[0];
    out << "UDS: SID = " <<  QString("0x%1").arg(SID, 2, 16, QLatin1Char( '0' )) << "\n";
    if(SID == FBL_NEGATIVE_RESPONSE) {
            out << "UDS Service: Negative Response\n";
            emit toConsole(*out.string());
            return;
    }
    bool response = (SID & FBL_SID_ACK);
    QString info = "";
    if(response) {
        info = "Response for ";
        SID -= FBL_SID_ACK;
    }

    switch(SID) {
        case FBL_DIAGNOSTIC_SESSION_CONTROL:
            out << info + "UDS Service: Diagnostic Session Control\n";
            break;
        case FBL_ECU_RESET:
            out << info + "UDS Service: ECU Reset\n";
            break;
        case FBL_SECURITY_ACCESS:
            out << info + "UDS Service: Security Access\n";
            break;
        case FBL_TESTER_PRESENT:
            out << info + "UDS Service: Tester Present\n";
            break;
        case FBL_READ_DATA_BY_IDENTIFIER:
            out << info + "UDS Service: Read Data By Identifier\n";
            out << "DID: " << QString("0x%1").arg(data[1], 2, 16, QLatin1Char( '0' )) << QString("0x%1").arg(data[2], 2, 16, QLatin1Char( '0' )) << "\n";
            if(response)
                out << "Data: " << QString::fromLocal8Bit(&data[3]) << "\n";
            break;
        case FBL_READ_MEMORY_BY_ADDRESS:
            out << info + "UDS Service: Read Memory By Address\n";
            break;
        case FBL_WRITE_DATA_BY_IDENTIFIER:
            out << info + "UDS Service: Write Data By Identifier\n";
            break;
        case FBL_REQUEST_DOWNLOAD:
            out << info + "UDS Service: Request Download\n";
            break;
        case FBL_REQUEST_UPLOAD:
            out << info + "UDS Service: Request Upload\n";
            break;
        case FBL_TRANSFER_DATA:
            out << info + "UDS Service: Transfer Data\n";
            break;
        case FBL_REQUEST_TRANSFER_EXIT:
            out << info + "UDS Service: Request Transfer Exit\n";
            break;
        default:
            out << info << "UDS Service: ERROR UNRECOGNIZED SSID\n";
            break;
    }

    emit toConsole(*out.string());
    
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
    emit toConsole("<< UDS: Sending out Request for Identification to all ECUs");

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
    emit toConsole("<< UDS: Sending out Diagnostic Session Control");

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
    emit toConsole("<< UDS: Sending out for ECU Reset");

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
    emit toConsole("<< UDS: Sending out Security Access for Seed");

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
    emit toConsole("<< UDS: Sending out Security Access for Verify Key");

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
    emit toConsole("<< UDS: Sending out Tester Present");

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
    emit toConsole("<< UDS: Sending out Read Data By Identifier");

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
    emit toConsole("<< UDS: Sending out Read Memory By Address");

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
    emit toConsole("<< UDS: Sending out Write Data By Identifier");

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
    emit toConsole("<< UDS: Sending out Request Download");

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
    emit toConsole("<< UDS: Sending Request Upload");

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
    emit toConsole("<< UDS: Sending out Transfer Data");

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
    emit toConsole("<< UDS: Sending out Request Transfer Exit");

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
    emit toConsole("<< UDS: Sending out Negative Response");

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

void UDS::rxDataReceiverSlot(const unsigned int id, const QByteArray &ba){
    qInfo("UDS: Slot - Received UDS Message to be processed");
    // Unwrap the data

    uint8_t* msg = (uint8_t*)calloc(ba.size(), sizeof(uint8_t));
    if (msg != nullptr){
        for(int i= 0; i < ba.size(); i++){
            msg[i] = ba[i];
            //qInfo() << "Step " << i << "Data: " << msg[i];
        }
        this->messageInterpreter(id, msg, ba.size());
        free(msg);
    }
}
