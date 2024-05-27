// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>

//============================================================================
// Name        : UDS.cpp
// Author      : Michael Bauerr, Wiktor Pilarczyk
// Version     : 0.3
// Copyright   : MIT
// Description : Qt UDS Layer implementation
//============================================================================

#include <QDebug>
#include <QDateTime>

#include "UDS.hpp"

#include "../UDS_Spec/uds_comm_spec.h"

UDS::UDS(){
    this->init = 0;
}

UDS::UDS(uint8_t gui_id) {
	this->gui_id = gui_id;
    this->init = 1;

    // Default: Sync-Mode is turned on
    this->synchronized_rx_tx = true;

    // Initialize the comm flag
    comm_mutex.lock();
    _comm = false;
    comm_mutex.unlock();
}

UDS::~UDS() {

}

/**
 * @brief Enables or Disables of the synchronized Mode for TX <-> RX
 * @param synchronized Switch to turn on sync mode or not
 */
void UDS::setSyncMode(bool synchronized){
    synchronized_rx_tx = synchronized;
}

//////////////////////////////////////////////////////////////////////////////
// Public - Receiving UDS Messages
//////////////////////////////////////////////////////////////////////////////
void UDS::messageInterpreter(unsigned int id, uint8_t *data, uint32_t no_bytes){

    QString s;
    QTextStream out(&s);

    rx_msg_valid = false;

    if(no_bytes == 0) {
        out << "UDS: No data passed";
        return;
    }

    // Checking on Negative Response
    uint8_t SID = data[0];
    out << "UDS: SID = " <<  QString("0x%1").arg(SID, 2, 16, QLatin1Char( '0' )) << " - ";
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
            out << info << "UDS Service: ERROR UNRECOGNIZED SID\n";
            break;
    }

    emit toConsole(*out.string());

    // Free the communication flag
    comm_mutex.lock();
    _comm = false;
    comm_mutex.unlock();
}


//////////////////////////////////////////////////////////////////////////////
// Public - Sending TX UDS Messages
//////////////////////////////////////////////////////////////////////////////


UDS::RESP UDS::reqIdentification() // Sending out broadcast for tester present
{
    if(!init){
        return NO_INIT;
    }

    // 1. Check on Free TX
    if(checkOnFreeTX() != TX_FREE) // Directly return if TX is still busy after wait time
        return STILL_BUSY;

    // 2. Lock the TX for own usage
    comm_mutex.lock();
    _comm = true;
    comm_mutex.unlock();

    // Info to Console
    qInfo("<< UDS: Sending out Request for Identification to all ECUs\n");
    emit toConsole("UDS: Sending out Request for Identification to all ECUs");

    // 3a. Setup the Communication Interface for Sending Message
	uint32_t id = (uint32_t)(FBLCAN_BASE_ADDRESS | this->gui_id);
	// First set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(id); // TODO: Check Architecture how to handle interface

    // 3b. Create the relevant message
	int len;  
    uint8_t *msg = _create_tester_present(&len, 0, 1); // Request Tester present from ECUs
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

    // 4. Transmit the data on the bus
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);

    // 5. Create the data that is expected
    rx_exp_data = nullptr;
    rx_no_bytes = 0;
    rx_exp_data = _create_tester_present(&rx_no_bytes, 1, 1);

    if(rx_exp_data == nullptr || rx_no_bytes == 0)
        return RX_ERROR;

    // 6. Wait on RX message interpreter
    RESP res = checkOnResponse(rx_max_waittime_long);
    if( res == TX_RX_OK){
        // Check on result of message interpreter
        if (rx_msg_valid)
            return TX_RX_OK;
        else
            return TX_RX_NOK;
    }
    else
        return res;
}


// Specification for Diagnostic and Communication Management
UDS::RESP UDS::diagnosticSessionControl(uint32_t id, uint8_t session){
    if(!init){
        return NO_INIT;
    }

    // 1. Check on Free TX
    if(checkOnFreeTX() != TX_FREE) // Directly return if TX is still busy after wait time
        return STILL_BUSY;

    // 2. Lock the TX for own usage
    comm_mutex.lock();
    _comm = true;
    comm_mutex.unlock();

    // Info to Console
    qInfo("<< UDS: Sending out Diagnostic Session Control\n");
    emit toConsole("<< UDS: Sending out Diagnostic Session Control");

    // 3a. Setup the Communication Interface for Sending Message
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

    // 3b. Create the relevant message
	int len;
	uint8_t *msg = _create_diagnostic_session_control(&len, 0, session);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

    // 4. Transmit the data on the bus
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);

    // 5. Create the data that is expected
    rx_exp_data = nullptr;
    rx_no_bytes = 0;
    rx_exp_data = _create_diagnostic_session_control(&rx_no_bytes, 1, session);

    if(rx_exp_data == nullptr || rx_no_bytes == 0)
        return RX_ERROR;


    // 6. Wait on RX message interpreter
    RESP res = checkOnResponse(rx_max_waittime_general);
    if(res == TX_RX_OK){
        // Check on result of message interpreter
        if (rx_msg_valid)
            return TX_RX_OK;
        else
            return TX_RX_NOK;
    }
    else
        return res;
}

UDS::RESP UDS::ecuReset(uint32_t id, uint8_t reset_type){
    if(!init){
        return NO_INIT;
    }

    // 1. Check on Free TX
    if(checkOnFreeTX() != TX_FREE) // Directly return if TX is still busy after wait time
        return STILL_BUSY;

    // 2. Lock the TX for own usage
    comm_mutex.lock();
    _comm = true;
    comm_mutex.unlock();

    // Info to Console
    qInfo("<< UDS: Sending out for ECU Reset\n");
    emit toConsole("<< UDS: Sending out for ECU Reset");

    // 3a. Setup the Communication Interface for Sending Message
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

    // 3b. Create the relevant message
    int len;
	uint8_t *msg = _create_ecu_reset(&len, 0, reset_type);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

    // 4. Transmit the data on the bus
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);

    // 5. Create the data that is expected
    rx_exp_data = nullptr;
    rx_no_bytes = 0;
    rx_exp_data = _create_ecu_reset(&rx_no_bytes, 1, reset_type);

    if(rx_exp_data == nullptr || rx_no_bytes == 0)
        return RX_ERROR;


    // 6. Wait on RX message interpreter
    RESP res = checkOnResponse(rx_max_waittime_general);
    if(res == TX_RX_OK){
        // Check on result of message interpreter
        if (rx_msg_valid)
            return TX_RX_OK;
        else
            return TX_RX_NOK;
    }
    else
        return res;
}

UDS::RESP UDS::securityAccessRequestSEED(uint32_t id){
    if(!init){
        return NO_INIT;
    }

    // 1. Check on Free TX
    if(checkOnFreeTX() != TX_FREE) // Directly return if TX is still busy after wait time
        return STILL_BUSY;

    // 2. Lock the TX for own usage
    comm_mutex.lock();
    _comm = true;
    comm_mutex.unlock();

    // Info to Console
    qInfo("<< UDS: Sending out Security Access for Seed\n");

    emit toConsole("<< UDS: Sending out Security Access for Seed");
    // 3a. Setup the Communication Interface for Sending Message
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

    // 3b. Create the relevant message
	int len;
	uint8_t *msg = _create_security_access(&len, 0, FBL_SEC_ACCESS_SEED, 0, 0);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

    // 4. Transmit the data on the bus
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);

    // 5. Create the data that is expected - Here: without any data included
    rx_exp_data = nullptr;
    rx_no_bytes = 0;
    rx_exp_data = _create_security_access(&rx_no_bytes, 1, FBL_SEC_ACCESS_SEED, 0, 0);

    if(rx_exp_data == nullptr || rx_no_bytes == 0)
        return RX_ERROR;


    // 6. Wait on RX message interpreter
    RESP res = checkOnResponse(rx_max_waittime_general);
    if(res == TX_RX_OK){
        // Check on result of message interpreter
        if (rx_msg_valid)
            return TX_RX_OK;
        else
            return TX_RX_NOK;
    }
    else
        return res;
}

UDS::RESP UDS::securityAccessVerifyKey(uint32_t id, uint8_t *key, uint8_t key_len){
    if(!init){
        return NO_INIT;
    }

    // 1. Check on Free TX
    if(checkOnFreeTX() != TX_FREE) // Directly return if TX is still busy after wait time
        return STILL_BUSY;

    // 2. Lock the TX for own usage
    comm_mutex.lock();
    _comm = true;
    comm_mutex.unlock();

    // Info to Console
    qInfo("<< UDS: Sending out Security Access for Verify Key\n");
    emit toConsole("<< UDS: Sending out Security Access for Verify Key");

    // 3a. Setup the Communication Interface for Sending Message
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

    // 3b. Create the relevant message
	int len;
	uint8_t *msg = _create_security_access(&len, 0, FBL_SEC_ACCESS_VERIFY_KEY, key, key_len);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

    // 4. Transmit the data on the bus
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);

    // 5. Create the data that is expected -> Here: As response data is not filled, but is expected in the response
    rx_exp_data = nullptr;
    rx_no_bytes = 0;
    rx_exp_data = _create_security_access(&rx_no_bytes, 1, FBL_SEC_ACCESS_VERIFY_KEY, 0, 0);

    if(rx_exp_data == nullptr || rx_no_bytes == 0)
        return RX_ERROR;


    // 6. Wait on RX message interpreter
    RESP res = checkOnResponse(rx_max_waittime_general);
    if(res == TX_RX_OK){
        // Check on result of message interpreter
        if (rx_msg_valid)
            return TX_RX_OK;
        else
            return TX_RX_NOK;
    }
    else
        return res;
}


UDS::RESP UDS::testerPresent(uint32_t id){
    if(!init){
        return NO_INIT;
    }

    // 1. Check on Free TX
    if(checkOnFreeTX() != TX_FREE) // Directly return if TX is still busy after wait time
        return STILL_BUSY;

    // 2. Lock the TX for own usage
    comm_mutex.lock();
    _comm = true;
    comm_mutex.unlock();

    // Info to Console
    qInfo("<< UDS: Sending out Tester Present\n");
    emit toConsole("<< UDS: Sending out Tester Present");

    // 3a. Setup the Communication Interface for Sending Message
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

    // 3b. Create the relevant message
	int len;
	uint8_t *msg = _create_tester_present(&len, 0, FBL_TESTER_PRES_WITHOUT_RESPONSE);
   // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

    // 4. Transmit the data on the bus
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);

    // 5. Create the data that is expected, here: Mainly no response is expected.
    rx_exp_data = nullptr;
    rx_no_bytes = 0;

    // Free the communication flag
    comm_mutex.lock();
    _comm = false;
    comm_mutex.unlock();

    if(synchronized_rx_tx)
        return TX_RX_OK;
    return TX_OK;
}

// Specification for Data Transmission
UDS::RESP UDS::readDataByIdentifier(uint32_t id, uint16_t identifier){
    if(!init){
        return NO_INIT;
    }

    // 1. Check on Free TX
    if(checkOnFreeTX() != TX_FREE) // Directly return if TX is still busy after wait time
        return STILL_BUSY;

    // 2. Lock the TX for own usage
    comm_mutex.lock();
    _comm = true;
    comm_mutex.unlock();

    // Info to Console
    qInfo("<< UDS: Sending out Read Data By Identifier\n");
    emit toConsole("<< UDS: Sending out Read Data By Identifier");

    // 3a. Setup the Communication Interface for Sending Message
    uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

    // 3b. Create the relevant message
	int len;
	uint8_t *msg = _create_read_data_by_ident(&len, 0, identifier, 0, 0);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

    // 4. Transmit the data on the bus
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);

    // 5. Create the data that is expected, Here: As response data is not filled, but is expected in the response
    rx_exp_data = nullptr;
    rx_no_bytes = 0;
    rx_exp_data = _create_read_data_by_ident(&rx_no_bytes, 1, identifier, 0, 0);

    if(rx_exp_data == nullptr || rx_no_bytes == 0)
        return RX_ERROR;


    // 6. Wait on RX message interpreter
    RESP res = checkOnResponse(rx_max_waittime_general);
    if(res == TX_RX_OK){
        // Check on result of message interpreter
        if (rx_msg_valid)
            return TX_RX_OK;
        else
            return TX_RX_NOK;
    }
    else
        return res;
}

UDS::RESP UDS::readMemoryByAddress(uint32_t id, uint32_t address, uint16_t no_bytes){
    if(!init){
        return NO_INIT;
    }

    // 1. Check on Free TX
    if(checkOnFreeTX() != TX_FREE) // Directly return if TX is still busy after wait time
        return STILL_BUSY;

    // 2. Lock the TX for own usage
    comm_mutex.lock();
    _comm = true;
    comm_mutex.unlock();

    // Info to Console
    qInfo("<< UDS: Sending out Read Memory By Address\n");
    emit toConsole("<< UDS: Sending out Read Memory By Address");

    // 3a. Setup the Communication Interface for Sending Message
    uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

    // 3b. Create the relevant message
    int len;
	uint8_t *msg = _create_read_memory_by_address(&len, 0, address, no_bytes, 0, 0);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

    // 4. Transmit the data on the bus
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);

    // 5. Create the data that is expected, Here: As response data is not filled, but is expected in the response
    rx_exp_data = nullptr;
    rx_no_bytes = 0;
    rx_exp_data = _create_read_memory_by_address(&rx_no_bytes, 1, address, no_bytes, 0, 0);

    if(rx_exp_data == nullptr || rx_no_bytes == 0)
        return RX_ERROR;

    // 6. Wait on RX message interpreter
    RESP res = checkOnResponse(rx_max_waittime_general);
    if(res == TX_RX_OK){
        // Check on result of message interpreter
        if (rx_msg_valid)
            return TX_RX_OK;
        else
            return TX_RX_NOK;
    }
    else
        return res;
}

UDS::RESP UDS::writeDataByIdentifier(uint32_t id, uint16_t identifier, uint8_t* data, uint8_t data_len){
    if(!init){
        return NO_INIT;
    }

    // 1. Check on Free TX
    if(checkOnFreeTX() != TX_FREE) // Directly return if TX is still busy after wait time
        return STILL_BUSY;

    // 2. Lock the TX for own usage
    comm_mutex.lock();
    _comm = true;
    comm_mutex.unlock();

    // Info to Console
    qInfo("<< UDS: Sending out Write Data By Identifier\n");
    emit toConsole("<< UDS: Sending out Write Data By Identifier");

    // 3a. Setup the Communication Interface for Sending Message
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

    // 3b. Create the relevant message
	int len;
	uint8_t *msg = _create_write_data_by_ident(&len, 0, identifier, data, data_len);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

    // 4. Transmit the data on the bus
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);

    // 5. Create the data that is expected, Here: No data is expected as response
    rx_exp_data = nullptr;
    rx_no_bytes = 0;
    rx_exp_data = _create_write_data_by_ident(&rx_no_bytes, 1, identifier, 0, 0);

    if(rx_exp_data == nullptr || rx_no_bytes == 0)
        return RX_ERROR;


    // 6. Wait on RX message interpreter
    RESP res = checkOnResponse(rx_max_waittime_general);
    if(res == TX_RX_OK){
        // Check on result of message interpreter
        if (rx_msg_valid)
            return TX_RX_OK;
        else
            return TX_RX_NOK;
    }
    else
        return res;
}

// Specification for Upload | Download
UDS::RESP UDS::requestDownload(uint32_t id, uint32_t address, uint32_t no_bytes){
    if(!init){
        return NO_INIT;
    }

    // 1. Check on Free TX
    if(checkOnFreeTX() != TX_FREE) // Directly return if TX is still busy after wait time
        return STILL_BUSY;

    // 2. Lock the TX for own usage
    comm_mutex.lock();
    _comm = true;
    comm_mutex.unlock();

    // Info to Console
    qInfo("<< UDS: Sending out Request Download\n");
    emit toConsole("<< UDS: Sending out Request Download");

    // 3a. Setup the Communication Interface for Sending Message
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

    // 3b. Create the relevant message
	int len;
	uint8_t *msg = _create_request_download(&len, 0, address, no_bytes);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

    // 4. Transmit the data on the bus
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);

    // 5. Create the data that is expected
    rx_exp_data = nullptr;
    rx_no_bytes = 0;
    rx_exp_data = _create_request_download(&rx_no_bytes, 1, address, no_bytes);

    if(rx_exp_data == nullptr || rx_no_bytes == 0)
        return RX_ERROR;


    // 6. Wait on RX message interpreter
    RESP res = checkOnResponse(rx_max_waittime_general);
    if(res == TX_RX_OK){
        // Check on result of message interpreter
        if (rx_msg_valid)
            return TX_RX_OK;
        else
            return TX_RX_NOK;
    }
    else
        return res;
}

UDS::RESP UDS::requestUpload(uint32_t id, uint32_t address, uint32_t no_bytes){
    if(!init){
        return NO_INIT;
    }

    // 1. Check on Free TX
    if(checkOnFreeTX() != TX_FREE) // Directly return if TX is still busy after wait time
        return STILL_BUSY;

    // 2. Lock the TX for own usage
    comm_mutex.lock();
    _comm = true;
    comm_mutex.unlock();

    // Info to Console
    qInfo("<< UDS: Sending Request Upload \n");
    emit toConsole("<< UDS: Sending Request Upload");

    // 3a. Setup the Communication Interface for Sending Message
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

    // 3b. Create the relevant message
	int len;
	uint8_t *msg = _create_request_upload(&len, 0, address, no_bytes);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

    // 4. Transmit the data on the bus
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);

    // 5. Create the data that is expected
    rx_exp_data = nullptr;
    rx_no_bytes = 0;
    rx_exp_data = _create_request_upload(&rx_no_bytes, 1, address, no_bytes);

    if(rx_exp_data == nullptr || rx_no_bytes == 0)
        return RX_ERROR;


    // 6. Wait on RX message interpreter
    RESP res = checkOnResponse(rx_max_waittime_general);
    if(res == TX_RX_OK){
        // Check on result of message interpreter
        if (rx_msg_valid)
            return TX_RX_OK;
        else
            return TX_RX_NOK;
    }
    else
        return res;
}

UDS::RESP UDS::transferData(uint32_t id, uint32_t address, uint8_t* data, uint8_t data_len){
    if(!init){
        return NO_INIT;
    }

    // 1. Check on Free TX
    if(checkOnFreeTX() != TX_FREE) // Directly return if TX is still busy after wait time
        return STILL_BUSY;

    // 2. Lock the TX for own usage
    comm_mutex.lock();
    _comm = true;
    comm_mutex.unlock();

    // Info to Console
    qInfo("<< UDS: Sending out Transfer Data\n");
    emit toConsole("<< UDS: Sending out Transfer Data");

    // 3a. Setup the Communication Interface for Sending Message
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

    // 3b. Create the relevant message
	int len;
    uint8_t *msg = _create_transfer_data(&len, address, data, data_len);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

    // 4. Transmit the data on the bus
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);

    // 5. Create the data that is expected, here: Mainly no response is expected.
    rx_exp_data = nullptr;
    rx_no_bytes = 0;

    // Free the communication flag
    comm_mutex.lock();
    _comm = false;
    comm_mutex.unlock();

    if(synchronized_rx_tx)
        return TX_RX_OK;
    return TX_OK;
}

UDS::RESP UDS::requestTransferExit(uint32_t id, uint32_t address){
    if(!init){
        return NO_INIT;
    }

    // 1. Check on Free TX
    if(checkOnFreeTX() != TX_FREE) // Directly return if TX is still busy after wait time
        return STILL_BUSY;

    // 2. Lock the TX for own usage
    comm_mutex.lock();
    _comm = true;
    comm_mutex.unlock();

    // Info to Console
    qInfo("<< UDS: Sending out Request Transfer Exit\n");
    emit toConsole("<< UDS: Sending out Request Transfer Exit");

    // 3a. Setup the Communication Interface for Sending Message
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

    // 3b. Create the relevant message
	int len;
	uint8_t *msg = _create_request_transfer_exit(&len, 0, address);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

    // 4. Transmit the data on the bus
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);

    // 5. Create the data that is expected
    rx_exp_data = nullptr;
    rx_no_bytes = 0;
    rx_exp_data = _create_request_transfer_exit(&rx_no_bytes, 1, address);

    if(rx_exp_data == nullptr || rx_no_bytes == 0)
        return RX_ERROR;


    // 6. Wait on RX message interpreter
    RESP res = checkOnResponse(rx_max_waittime_general);
    if(res == TX_RX_OK){
        // Check on result of message interpreter
        if (rx_msg_valid)
            return TX_RX_OK;
        else
            return TX_RX_NOK;
    }
    else
        return res;
}

// Supported Common Response Codes
UDS::RESP UDS::negativeResponse(uint32_t id, uint8_t reg_sid, uint8_t neg_resp_code){
    if(!init){
        return NO_INIT;
    }

    // 1. Check on Free TX
    if(checkOnFreeTX() != TX_FREE) // Directly return if TX is still busy after wait time
        return STILL_BUSY;

    // 2. Lock the TX for own usage
    comm_mutex.lock();
    _comm = true;
    comm_mutex.unlock();

    // Info to Console
    qInfo("<< UDS: Sending out Negative Response\n");
    emit toConsole("<< UDS: Sending out Negative Response");

    // 3a. Setup the Communication Interface for Sending Message
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
	// Set the right ID to be used for transmitting
    qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

    // 3b. Create the relevant message
	int len;
	uint8_t *msg = _create_neg_response(&len, reg_sid, neg_resp_code);
    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

    // 4. Transmit the data on the bus
    qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);

    // 5. Create the data that is expected, here: Mainly no response is expected.
    rx_exp_data = nullptr;
    rx_no_bytes = 0;

    // Free the communication flag
    comm_mutex.lock();
    _comm = false;
    comm_mutex.unlock();

    if(synchronized_rx_tx)
        return TX_RX_OK;
    return TX_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////////

/**
 * @brief Creates the common ID based on the UDS Communication Specification
 * @param base_id Given Base ID
 * @param gui_id Given GUI ID
 * @param ecu_id Given ECU ID
 * @return
 */
uint32_t UDS::createCommonID(uint32_t base_id, uint8_t gui_id, uint32_t ecu_id){

	if (ecu_id > 0xFFF){
        qInfo("ID is not supported");
		return 0;
	}

	uint32_t send_id = (uint32_t)(base_id | gui_id);
	send_id |= (ecu_id<<4); // Combine ECU ID with send ID including GUI ID (see spec)

	return send_id;
}

/**
 * @brief Checks if the communication is free to send and receive message
 * @return STILL_BUSY if the communication is not free after max waiting time, TX_FREE if TX can be send
 */
UDS::RESP UDS::checkOnFreeTX(){

    bool isCommBusy = true;
    QDateTime start = QDateTime::currentDateTime();
    do {
        comm_mutex.lock();
        isCommBusy = _comm;
        comm_mutex.unlock();
        if (start.msecsTo(QDateTime::currentDateTime()) > tx_max_waittime_free_tx){
            return STILL_BUSY;
        }
    } while(isCommBusy);
    return TX_FREE;
}

UDS::RESP UDS::checkOnResponse(uint32_t waittime){
    // No synchronizing of TX to RX
    if(!synchronized_rx_tx){
        comm_mutex.lock();
        _comm = false;
        comm_mutex.unlock();

        return TX_OK;
    }

    bool isCommBusy = true;
    QDateTime start = QDateTime::currentDateTime();
    do {
        comm_mutex.lock();
        isCommBusy = _comm;
        comm_mutex.unlock();

        if (start.msecsTo(QDateTime::currentDateTime()) > waittime){
            // Free the communication flag
            comm_mutex.lock();
            _comm = false;
            comm_mutex.unlock();

            return RX_NO_RESPONSE;
        }
    } while(isCommBusy);

    return TX_RX_OK;
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
