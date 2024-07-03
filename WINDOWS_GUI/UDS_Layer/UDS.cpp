// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>

//============================================================================
// Name        : UDS.cpp
// Author      : Michael Bauer, Wiktor Pilarczyk
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
    this->ecu_rec_buffer_size = 0;

    // Default: Sync-Mode is turned on
    this->synchronized_rx_tx = true;

    // Initialize the comm flag
    comm_mutex.lock();
    _comm = false;
    comm_mutex.unlock();
}

UDS::~UDS() {

}

//////////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////////

/**
 * @brief Enables or Disables of the synchronized Mode for TX <-> RX
 * @param synchronized Switch to turn on sync mode or not
 */
void UDS::setSyncMode(bool synchronized){
    synchronized_rx_tx = synchronized;
}

uint32_t UDS::getECUTransferDataBufferSize(){
    return ecu_rec_buffer_size;
}

uint32_t UDS::getECUChecksum() {
    return ecu_rec_checksum;
}

//////////////////////////////////////////////////////////////////////////////
// Private - Receiving UDS Messages
//////////////////////////////////////////////////////////////////////////////

/**
 * @brief Internal method to interpret incoming messages. The messages are checked based on setup during UDS TX methods
 * @param id Received ID of the Sender
 * @param data Received Data of the Sender
 * @param no_bytes Received number of bytes of the data
 */

static inline const bool rxMsgValid(const bool neg_resp, const bool eq, const uint32_t rx_no_bytes, const uint32_t no_bytes,  const uint8_t* const rx_exp_data,const uint8_t* const data, const size_t n) {
    bool ans = !neg_resp;
    ans &= eq ? rx_no_bytes == no_bytes : rx_no_bytes < no_bytes;
    for(size_t i = 1; i < n + 1; ++i) 
        ans &= rx_exp_data[i] == data[i];
    return ans;
}

void UDS::rxMsgCopyToBuffer(uint8_t* data, int len){
    for(int i = 0; i < len && i < RX_EXP_DATA_BUFFER_SIZE; i++) {
        rx_exp_data[i] = data[i];
    }
}

void UDS::messageInterpreter(unsigned int id, uint8_t *data, uint32_t no_bytes){

    // Initialize the Msg flags
    rx_msg_valid = false;
    rx_msg_neg_resp = false;

    QString s;
    QTextStream out(&s);

    if(no_bytes == 0) {
        out << "UDS: No data passed\n";
        emit toConsole(*out.string());
        return;
    }

    // 1. Checking on Negative Response
    uint8_t SID = data[0];
    bool neg_resp = false;
    QString neg_resp_code = "";
    if(SID == FBL_NEGATIVE_RESPONSE && no_bytes >= 3) {
        neg_resp = true;
        rx_msg_neg_resp = true;
        SID = data[1];
        neg_resp_code = translateNegResp(data[2]);
        out << "Negative Response (Negative Response Code:" << neg_resp_code << ")\n";
    }

    // 2. Do a precheck of the message, Ignore if SID does not fit
    if((!neg_resp) && ((rx_no_bytes <= 0) || data[0] != rx_exp_data[0])){
        qInfo() << ">> UDS INFO: Ignoring message - Received SID "<<QString("0x%1").arg(uint8_t(data[0]), 2, 16, QLatin1Char( '0' )) << " does not fit to expected SID "<< QString("0x%1").arg(uint8_t(rx_exp_data[0]), 2, 16, QLatin1Char( '0' ));
        return;
    }

    // 3. Check on the actual message
    bool response = (SID & FBL_SID_ACK);
    QString info = "";
    if(response) {
        info = ">> UDS: "; // Indicates Response
        SID -= FBL_SID_ACK;
    }
    QString SID_str = " (" + QString("0x%1").arg(SID, 2, 16, QLatin1Char( '0' )) +")";
    QString ID_str = " from ID "+ QString("0x%1").arg(id, 8, 16, QLatin1Char( '0' ));
    QString Key = QString::number(id)+"#"+QString::number(SID);
    QMap<QString, QString> signalContent;
    signalContent[Key] = "";
    uint16_t did_raw;
    QString DID = "";
    QString read_data;

    switch(SID) {
        case FBL_DIAGNOSTIC_SESSION_CONTROL:
            out << info + "Diagnostic Session Control" << SID_str << ID_str<<"\n";

            // Check on the relevant message - Session is correct
            rx_msg_valid = rxMsgValid(neg_resp, true, rx_no_bytes, no_bytes, rx_exp_data, data, 1);
            break;

        case FBL_ECU_RESET:
            out << info + "ECU Reset " << SID_str << ID_str<<"\n";
            // Check on the relevant message - ECU Reset Type is correct
            rx_msg_valid = rxMsgValid(neg_resp, true, rx_no_bytes, no_bytes, rx_exp_data, data, 1);
            break;

        case FBL_SECURITY_ACCESS:
            out << info + "Security Access "<< SID_str << ID_str<<"\n";
            // Check on the relevant message - Request Type is correct
            rx_msg_valid = rxMsgValid(neg_resp, true, rx_no_bytes, no_bytes, rx_exp_data, data, 1);
            break;

        case FBL_TESTER_PRESENT:
            out << info + "Tester Present " << SID_str << ID_str<<"\n";
            // Check on the relevant message - Response Type is correct
            rx_msg_valid = rxMsgValid(neg_resp, true, rx_no_bytes, no_bytes, rx_exp_data, data, 1);
            break;

        case FBL_READ_DATA_BY_IDENTIFIER:
            out << info + "Read Data By Identifier "<< SID_str << ID_str<<"\n";
            did_raw = (data[1]<<8)|data[2];
            DID = QString("0x%1").arg((data[1]<<8)|data[2], 2, 16, QLatin1Char( '0' ));
            read_data = readDIDData(did_raw, data+3, no_bytes - 3);
            out << "DID: " << translateDID(did_raw) << " (" << DID << ")\n";
            if(response){
                out << "Data: " << read_data << "\n";
            }

            // Check on the relevant message - Data is included, DID is correct
            rx_msg_valid = rxMsgValid(neg_resp, false, rx_no_bytes, no_bytes, rx_exp_data, data, 2);
            signalContent[Key] = QString::number(did_raw)+"#"+read_data;
            break;

        case FBL_READ_MEMORY_BY_ADDRESS:
            out << info + "Read Memory By Address "<< SID_str << ID_str<<"\n";

            // Check on the relevant message - Data is included, Adress is correct
            rx_msg_valid = rxMsgValid(neg_resp, false, rx_no_bytes, no_bytes, rx_exp_data, data, 4);
            break;

        case FBL_WRITE_DATA_BY_IDENTIFIER:
            out << info + "Write Data By Identifier "<< SID_str << ID_str<<"\n";

            // Check on the relevant message - DID is correct
            rx_msg_valid = rxMsgValid(neg_resp, true, rx_no_bytes, no_bytes, rx_exp_data, data, 2);
            break;

        case FBL_REQUEST_DOWNLOAD:
            out << info + "Request Download "<< SID_str << ID_str<<"\n";

            // Check on the relevant message - Adress is correct
            rx_msg_valid = rxMsgValid(neg_resp, true, rx_no_bytes, no_bytes, rx_exp_data, data, 4);
            if(rx_msg_valid){
                this->ecu_rec_buffer_size = 0;
                this->ecu_rec_buffer_size |= (data[5] << 24);
                this->ecu_rec_buffer_size |= (data[6] << 16);
                this->ecu_rec_buffer_size |= (data[7] << 8);
                this->ecu_rec_buffer_size |= data[8];

                out << "ECU Buffer Size in bytes: "+QString::number(ecu_rec_buffer_size);
            }
            else
                this->ecu_rec_buffer_size = 0;

            break;

        case FBL_REQUEST_UPLOAD:
            out << info + "Request Upload "<< SID_str << ID_str<<"\n";

            // Check on the relevant message - Adress is correct
            rx_msg_valid = rxMsgValid(neg_resp, true, rx_no_bytes, no_bytes, rx_exp_data, data, 4);
            if (rx_msg_valid) {
                this->ecu_rec_checksum = 0;
                this->ecu_rec_checksum |= (data[5] << 24);
                this->ecu_rec_checksum |= (data[6] << 16);
                this->ecu_rec_checksum |= (data[7] << 8);
                this->ecu_rec_checksum |= data[8];
            } else {
                this->ecu_rec_checksum = 0;
            }
            //qInfo() << QString("0x%1").arg(ecu_rec_checksum, 2, 16, QLatin1Char( '0' ));
            break;

        case FBL_TRANSFER_DATA:
            out << info + "Transfer Data "<< SID_str << ID_str<<"\n";

            // Check on the relevant message - Adress is correct
            rx_msg_valid = rxMsgValid(neg_resp, true, rx_no_bytes, no_bytes, rx_exp_data, data, 4);
            break;

        case FBL_REQUEST_TRANSFER_EXIT:
            out << info + "Request Transfer Exit "<< SID_str << ID_str<<"\n";

            // Info: Response includes the end address. There is no content check here
            rx_msg_valid = true;
            break;

        default:
            if(!neg_resp)
                out << info << "ERROR UNRECOGNIZED SID "<< SID_str << ID_str<<"\n";
            break;
    }

    qInfo() << *out.string();
    emit toConsole(*out.string());

    // Only release
    if (rx_msg_valid) {
        // Release the communication flag
        comm_mutex.lock();
        _comm = false;
        comm_mutex.unlock();

        // Signal the response
        emit ecuResponse(signalContent);
    }
}


//////////////////////////////////////////////////////////////////////////////
// Public - Sending TX UDS Messages
//////////////////////////////////////////////////////////////////////////////

const UDS::RESP UDS::txMessageStart() {
    if(!init) {
        return NO_INIT;
    }

    // 1. Check on Free TX
    if(checkOnFreeTX() != TX_FREE) // Directly return if TX is still busy after wait time
        return STILL_BUSY;

    // 2. Lock the TX for own usage
    comm_mutex.lock();
    _comm = true;
    comm_mutex.unlock();
    return TX_OK;
}

void UDS::txMessageSend(uint32_t id, uint8_t *msg, int len) {
    // 3. Setup the Communication Interface for Sending Message
	// Set the right ID to be used for transmitting
    if(VERBOSE_UDS) qInfo("UDS: Sending Signal setID");
    emit setID(id); // TODO: Check Architecture how to handle interface

    // Wrap data into QByteArray
    QByteArray qbdata;
    qbdata.resize(len);
    for(int i=0; i < qbdata.size(); i++)
        qbdata[i] = msg[i];
    // Free the allocated memory of msg
    free(msg);

    // 4. Transmit the data on the bus
    if(VERBOSE_UDS) qInfo() << "UDS: Sending Signal txData with " << len << " bytes";
    emit txData(qbdata);
}

const UDS::RESP UDS::rxMessageValid(uint32_t waittime) {
    if(rx_no_bytes == 0)
        return RX_ERROR;

    // 5. Wait on RX message interpreter
    RESP res = checkOnResponse(waittime);
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

/**
 * @brief Method to send a broadcast to all ECUs on the bus, so that they respond with Tester Present
 * @return UDS::RESP accordingly
 */
UDS::RESP UDS::reqIdentification() { // broadcast for tester present
    UDS::RESP resp = txMessageStart();
    if(resp != TX_OK){
        return resp;
    }

	uint32_t id = (uint32_t)(FBLCAN_BASE_ADDRESS | this->gui_id);
    QString id_str = " using ID "+ QString("0x%1").arg(id, 8, 16, QLatin1Char( '0' ));

    // Info to Console
    qInfo() << "<< UDS: Request for Identification to all ECUs" <<id_str <<"\n";
    emit toConsole("UDS: Request for Identification to all ECUs" + id_str);

    int len;  
    uint8_t *msg = _create_tester_present(&len, 0, 1); // Request Tester present from ECUs
    txMessageSend(id, msg, len);    

    rx_no_bytes = 0;
    uint8_t *temp_rx_exp_data = _create_tester_present(&rx_no_bytes, 1, 1);
    rxMsgCopyToBuffer(temp_rx_exp_data, rx_no_bytes);
    free(temp_rx_exp_data);

    // Release the communication flag
    comm_mutex.lock();
    _comm = false;
    comm_mutex.unlock();

    return TX_OK;
}


// Specification for Diagnostic and Communication Management
/**
 * @brief Method to switch the Diagnostic Session of a given ECU ID to a given Session
 * @param id Target ID
 * @param session Target Session
 * @return UDS::RESP accordingly
 */
UDS::RESP UDS::diagnosticSessionControl(uint32_t id, uint8_t session) {
    UDS::RESP resp = txMessageStart();
    if(resp != TX_OK){
        return resp;
    }

	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
    QString id_str = " using ID "+ QString("0x%1").arg(send_id, 8, 16, QLatin1Char( '0' ));

    // Info to Console
    qInfo("<< UDS: Diagnostic Session Control\n");
    emit toConsole("<< UDS: Diagnostic Session Control" + id_str);

	int len;
	uint8_t *msg = _create_diagnostic_session_control(&len, 0, session);
    txMessageSend(send_id, msg, len);

    rx_no_bytes = 0;
    uint8_t *temp_rx_exp_data = _create_diagnostic_session_control(&rx_no_bytes, 1, session);
    rxMsgCopyToBuffer(temp_rx_exp_data, rx_no_bytes);
    free(temp_rx_exp_data);
    return rxMessageValid(rx_max_waittime_general);
}

/**
 * @brief Method to request a reset with the given type for the given ECU ID
 * @param id Target ID
 * @param reset_type Target Reset Type
 * @return UDS::RESP accordingly
 */
UDS::RESP UDS::ecuReset(uint32_t id, uint8_t reset_type) {
    UDS::RESP resp = txMessageStart();
    if(resp != TX_OK){
        return resp;
    }

	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
    QString id_str = " using ID "+ QString("0x%1").arg(send_id, 8, 16, QLatin1Char( '0' ));

    // Info to Console
    qInfo("<< UDS: for ECU Reset\n");
    emit toConsole("<< UDS: for ECU Reset" + id_str);

    int len;
	uint8_t *msg = _create_ecu_reset(&len, 0, reset_type);
    txMessageSend(send_id, msg, len);

    rx_no_bytes = 0;
    uint8_t *temp_rx_exp_data = _create_ecu_reset(&rx_no_bytes, 1, reset_type);
    rxMsgCopyToBuffer(temp_rx_exp_data, rx_no_bytes);
    free(temp_rx_exp_data);
    return rxMessageValid(rx_max_waittime_general);
}

/**
 * @brief Method to request a security access SEED from the given ECU ID. First step for Security Access
 * @param id Target ID
 * @return UDS::RESP accordingly
 */
UDS::RESP UDS::securityAccessRequestSEED(uint32_t id) {
    UDS::RESP resp = txMessageStart();
    if(resp != TX_OK){
        return resp;
    }

	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
    QString id_str = " using ID "+ QString("0x%1").arg(send_id, 8, 16, QLatin1Char( '0' ));

    // Info to Console
    qInfo("<< UDS: Security Access for Seed\n");
    emit toConsole("<< UDS: Security Access for Seed" + id_str);

	int len;
	uint8_t *msg = _create_security_access(&len, 0, FBL_SEC_ACCESS_SEED, 0, 0);
    txMessageSend(send_id, msg, len);

    rx_no_bytes = 0;
    uint8_t *temp_rx_exp_data = _create_security_access(&rx_no_bytes, 1, FBL_SEC_ACCESS_SEED, 0, 0);
    rxMsgCopyToBuffer(temp_rx_exp_data, rx_no_bytes);
    free(temp_rx_exp_data);
    return rxMessageValid(rx_max_waittime_general);
}

/**
 * @brief Method to send the calculated Key with given Len to the given ECU ID. Second step for Security Access
 * @param id Target ID
 * @param key Calculated key to be transmitted
 * @param key_len Length of the calculated key
 * @return UDS::RESP accordingly
 */
UDS::RESP UDS::securityAccessVerifyKey(uint32_t id, uint8_t *key, uint8_t key_len) {
    UDS::RESP resp = txMessageStart();
    if(resp != TX_OK){
        return resp;
    }

	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
    QString id_str = " using ID "+ QString("0x%1").arg(send_id, 8, 16, QLatin1Char( '0' ));

    // Info to Console
    qInfo("<< UDS: Security Access for Verify Key\n");
    emit toConsole("<< UDS: Security Access for Verify Key" + id_str);

	int len;
	uint8_t *msg = _create_security_access(&len, 0, FBL_SEC_ACCESS_VERIFY_KEY, key, key_len);
    txMessageSend(send_id, msg, len);

    rx_no_bytes = 0;
    uint8_t *temp_rx_exp_data = _create_security_access(&rx_no_bytes, 1, FBL_SEC_ACCESS_VERIFY_KEY, 0, 0);
    rxMsgCopyToBuffer(temp_rx_exp_data, rx_no_bytes);
    free(temp_rx_exp_data);
    return rxMessageValid(rx_max_waittime_general);
}

/**
 * @brief Method to send a Tester Present to the given ECU ID. Message is send without any expected response
 * @param id Target ID
 * @return UDS::RESP accordingly
 */
UDS::RESP UDS::testerPresent(uint32_t id) {
    UDS::RESP resp = txMessageStart();
    if(resp != TX_OK){
        return resp;
    }

	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
    QString id_str = " using ID "+ QString("0x%1").arg(send_id, 8, 16, QLatin1Char( '0' ));

    // Info to Console
    qInfo("<< UDS: Tester Present without Response\n");
    emit toConsole("<< UDS: Tester Present without Response" + id_str);

	int len;
	uint8_t *msg = _create_tester_present(&len, 0, FBL_TESTER_PRES_WITHOUT_RESPONSE);
    txMessageSend(send_id, msg, len);

    // Create the data that is expected, here: Mainly no response is expected.
    rx_no_bytes = 0;

    // Release the communication flag
    comm_mutex.lock();
    _comm = false;
    comm_mutex.unlock();

    if(synchronized_rx_tx)
        return TX_RX_OK;
    return TX_OK;
}


/**
 * @brief Method to send a Tester Present to the given ECU ID. Message is send with expected response
 * @param id Target ID
 * @return UDS::RESP accordingly
 */
UDS::RESP UDS::testerPresentResponse(uint32_t id) {
    UDS::RESP resp = txMessageStart();
    if(resp != TX_OK){
        return resp;
    }

	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
    QString id_str = " using ID "+ QString("0x%1").arg(send_id, 8, 16, QLatin1Char( '0' ));

    // Info to Console
    qInfo("<< UDS: Tester Present with Response\n");
    emit toConsole("<< UDS: Tester Present with Response" + id_str);

	int len;
	uint8_t *msg = _create_tester_present(&len, 0, FBL_TESTER_PRES_WITH_RESPONSE);
    txMessageSend(send_id, msg, len);

    rx_no_bytes = 0;
    uint8_t *temp_rx_exp_data = _create_tester_present(&rx_no_bytes, 1, FBL_TESTER_PRES_WITH_RESPONSE);
    rxMsgCopyToBuffer(temp_rx_exp_data, rx_no_bytes);
    free(temp_rx_exp_data);
    return rxMessageValid(rx_max_waittime_general);
}

// Specification for Data Transmission
/**
 * @brief Method to request Data by a given Identifier
 * @param id Target ID
 * @param identifier Data Identifier
 * @return UDS::RESP accordingly
 */
UDS::RESP UDS::readDataByIdentifier(uint32_t id, uint16_t identifier) {
    UDS::RESP resp = txMessageStart();
    if(resp != TX_OK){
        return resp;
    }

    uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
    QString id_str = " using ID "+ QString("0x%1").arg(send_id, 8, 16, QLatin1Char( '0' ));
    QString did_str = " for DID "+translateDID(identifier) + " ("+QString("0x%1").arg(identifier, 4, 16, QLatin1Char( '0' )) +")";

    // Info to Console
    qInfo("<< UDS: Read Data By Identifier\n");
    emit toConsole("<< UDS: Read Data By Identifier" + id_str + did_str);
    
	int len;
	uint8_t *msg = _create_read_data_by_ident(&len, 0, identifier, 0, 0);
    txMessageSend(send_id, msg, len);

    // 5. Create the data that is expected, Here: As response data is not filled, but is expected in the response
    rx_no_bytes = 0;
    uint8_t *temp_rx_exp_data = _create_read_data_by_ident(&rx_no_bytes, 1, identifier, 0, 0);
    rxMsgCopyToBuffer(temp_rx_exp_data, rx_no_bytes);
    free(temp_rx_exp_data);
    return rxMessageValid(rx_max_waittime_general);
}

/**
 * @brief Method to request Data by a given address from a given ECU ID. The number of bytes is specified with the parameter no_bytes
 * @param id Target ID
 * @param address Memory Address to be read
 * @param no_bytes Number of bytes to be read from given Memory Address
 * @return UDS::RESP accordingly
 */
UDS::RESP UDS::readMemoryByAddress(uint32_t id, uint32_t address, uint16_t no_bytes) {
    UDS::RESP resp = txMessageStart();
    if(resp != TX_OK){
        return resp;
    }

    uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
    QString id_str = " using ID "+ QString("0x%1").arg(send_id, 8, 16, QLatin1Char( '0' ));

    // Info to Console
    qInfo("<< UDS: Read Memory By Address\n");
    emit toConsole("<< UDS: Read Memory By Address" + id_str);

    int len;
	uint8_t *msg = _create_read_memory_by_address(&len, 0, address, no_bytes, 0, 0);
    txMessageSend(send_id, msg, len);

    rx_no_bytes = 0;
    uint8_t *temp_rx_exp_data = _create_read_memory_by_address(&rx_no_bytes, 1, address, no_bytes, 0, 0);
    rxMsgCopyToBuffer(temp_rx_exp_data, rx_no_bytes);
    free(temp_rx_exp_data);
    return rxMessageValid(rx_max_waittime_general);
}

/**
 * @brief Method to write Data by a given Identifier
 * @param id Target ID
 * @param identifier Data Identifier
 * @param data Given Data to be written
 * @param data_len Length of the given data
 * @return UDS::RESP accordingly
 */
UDS::RESP UDS::writeDataByIdentifier(uint32_t id, uint16_t identifier, uint8_t* data, uint8_t data_len) {
    UDS::RESP resp = txMessageStart();
    if(resp != TX_OK){
        return resp;
    }

	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
    QString id_str = " using ID "+ QString("0x%1").arg(send_id, 8, 16, QLatin1Char( '0' ));

    // Info to Console
    qInfo("<< UDS: Write Data By Identifier\n");
    emit toConsole("<< UDS: Write Data By Identifier" + id_str);

	int len;
	uint8_t *msg = _create_write_data_by_ident(&len, 0, identifier, data, data_len);
    txMessageSend(send_id, msg, len);  

    rx_no_bytes = 0;
    uint8_t *temp_rx_exp_data = _create_write_data_by_ident(&rx_no_bytes, 1, identifier, 0, 0);
    rxMsgCopyToBuffer(temp_rx_exp_data, rx_no_bytes);
    free(temp_rx_exp_data);
    return rxMessageValid(rx_max_waittime_general);
}

// Specification for Upload | Download
/**
 * @brief Method to request a Download for the given Memory Address for the given number of byte. First Step for Data Transfer
 * @param id Target ID
 * @param address Target Memory Address
 * @param no_bytes Number of bytes to be downloaded to ECU ID
 * @return UDS::RESP accordingly
 */
UDS::RESP UDS::requestDownload(uint32_t id, uint32_t address, uint32_t no_bytes) {
    UDS::RESP resp = txMessageStart();
    if(resp != TX_OK){
        return resp;
    }

	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
    QString id_str = " using ID "+ QString("0x%1").arg(send_id, 8, 16, QLatin1Char( '0' ));

    // Info to Console
    qInfo("<< UDS: Request Download\n");
    emit toConsole("<< UDS: Request Download" + id_str);

	int len;
	uint8_t *msg = _create_request_download(&len, 0, address, no_bytes);
    txMessageSend(send_id, msg, len);  

    rx_no_bytes = 0;
    uint8_t *temp_rx_exp_data = _create_request_download(&rx_no_bytes, 1, address, 0); // ECU need to response with the buffer size for bytes_size
    rxMsgCopyToBuffer(temp_rx_exp_data, rx_no_bytes);
    free(temp_rx_exp_data);
    return rxMessageValid(rx_max_waittime_general);
}

/**
 * @brief Method to request a Upload for the given Memory Address for the given number of bytes. First Step for Data Transfer
 * @param id Target ID
 * @param address Target Memory Address
 * @param no_bytes Number of bytes to be uploaded from ECU ID
 * @return UDS::RESP accordingly
 */
UDS::RESP UDS::requestUpload(uint32_t id, uint32_t address, uint32_t no_bytes) {
    UDS::RESP resp = txMessageStart();
    if(resp != TX_OK){
        return resp;
    }

	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
    QString id_str = " using ID "+ QString("0x%1").arg(send_id, 8, 16, QLatin1Char( '0' ));

    // Info to Console
    qInfo("<< UDS: Sending Request Upload \n");
    emit toConsole("<< UDS: Sending Request Upload" + id_str);

	int len;
    uint8_t *msg = _create_request_upload(&len, 0, address, no_bytes);
    txMessageSend(send_id, msg, len);  

    rx_no_bytes = 0;
    uint8_t *temp_rx_exp_data = _create_request_upload(&rx_no_bytes, 1, address, 0); // ECU need to response with the buffer size for bytes_size
    rxMsgCopyToBuffer(temp_rx_exp_data, rx_no_bytes);
    free(temp_rx_exp_data);
    return rxMessageValid(rx_max_waittime_general);
}

/**
 * @brief Method to transfer the given data to the given ECU ID. Second step for Data Transmission
 * @param id Target ID
 * @param address Target Memory Address
 * @param data Data to be transferred
 * @param data_len Number of bytes of the data
 * @return UDS::RESP accordingly
 */
UDS::RESP UDS::transferData(uint32_t id, uint32_t address, uint8_t* data, uint32_t data_len) {
    UDS::RESP resp = txMessageStart();
    if(resp != TX_OK){
        return resp;
    }

	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
    QString id_str = " using ID "+ QString("0x%1").arg(send_id, 8, 16, QLatin1Char( '0' ));

    // Info to Console
    qInfo("<< UDS: Transfer Data\n");
    emit toConsole("<< UDS: Transfer Data" + id_str);

	int len;
    uint8_t *msg = _create_transfer_data(&len, 0, address, data, data_len);
    txMessageSend(send_id, msg, len);  

    // Create the data that is expected
    rx_no_bytes = 0;
    uint8_t *temp_rx_exp_data = _create_transfer_data(&rx_no_bytes, 1, address, 0, 0);
    rxMsgCopyToBuffer(temp_rx_exp_data, rx_no_bytes);
    free(temp_rx_exp_data);
    return rxMessageValid(rx_max_waittime_flashing);
}

/**
 * @brief Method to exit the transfer to the given ECU ID. Third and last step for Data Transmission
 * @param id Target ID
 * @param address Target Memory Address
 * @return UDS::RESP accordingly
 */
UDS::RESP UDS::requestTransferExit(uint32_t id, uint32_t address) {
    UDS::RESP resp = txMessageStart();
    if(resp != TX_OK){
        return resp;
    }

	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
    QString id_str = " using ID "+ QString("0x%1").arg(send_id, 8, 16, QLatin1Char( '0' ));

    // Info to Console
    qInfo("<< UDS: Request Transfer Exit\n");
    emit toConsole("<< UDS: Request Transfer Exit" + id_str);

	int len;
	uint8_t *msg = _create_request_transfer_exit(&len, 0, address);
    txMessageSend(send_id, msg, len);  

    rx_no_bytes = 0;
    uint8_t *temp_rx_exp_data = _create_request_transfer_exit(&rx_no_bytes, 1, address);
    rxMsgCopyToBuffer(temp_rx_exp_data, rx_no_bytes);
    free(temp_rx_exp_data);
    return rxMessageValid(rx_max_waittime_general);
}

// Supported Common Response Codes
/**
 * @brief Method to send a Negative Response to the given ECU ID. This is used as response to a request
 * @param id Target ID
 * @param rej_sid Rejected SID
 * @param neg_resp_code Negative Response Code (NRC)
 * @return UDS::RESP accordingly
 */
UDS::RESP UDS::negativeResponse(uint32_t id, uint8_t rej_sid, uint8_t neg_resp_code) {
    UDS::RESP resp = txMessageStart();
    if(resp != TX_OK){
        return resp;
    }

    // 3a. Setup the Communication Interface for Sending Message
	uint32_t send_id = createCommonID((uint32_t)FBLCAN_BASE_ADDRESS, this->gui_id, id);
    QString id_str = " using ID "+ QString("0x%1").arg(send_id, 8, 16, QLatin1Char( '0' ));

    // Info to Console
    qInfo("<< UDS: Negative Response\n");
    emit toConsole("<< UDS: Negative Response" + id_str);

	// Set the right ID to be used for transmitting
    if(VERBOSE_UDS) qInfo("UDS: Sending Signal setID");
    emit setID(send_id);

    // 3b. Create the relevant message
	int len;
    uint8_t *msg = _create_neg_response(&len, rej_sid, neg_resp_code);
    txMessageSend(send_id, msg, len);  

    // 5. Create the data that is expected, here: Mainly no response is expected.
    rx_no_bytes = 0;

    // Release the communication flag
    comm_mutex.lock();
    _comm = false;
    comm_mutex.unlock();

    if(synchronized_rx_tx)
        return TX_RX_OK;
    return TX_OK;
}

/**
 * @brief Translates a given Negative Response Code into a String representation according to UDS Communication documentation
 * @param nrc Given Negative Response Code for translation
 * @return
 */
QString UDS::translateNegResp(uint8_t nrc){
    switch(nrc){
    case FBL_RC_GENERAL_REJECT:
        return QString("General reject"); break;
    case FBL_RC_SERVICE_NOT_SUPPORTED:
        return QString("Service not supported"); break;
    case FBL_RC_SUB_FUNC_NOT_SUPPORTED:
        return QString("Sub-Function not supported"); break;
    case FBL_RC_INCORRECT_MSG_LEN_OR_INV_FORMAT:
        return QString("Incorrect msg len or invalid format"); break;
    case FBL_RC_RESPONSE_TOO_LONG:
        return QString("Response too long"); break;
    case FBL_RC_BUSY_REPEAT_REQUEST:
        return QString("Busy repeat request"); break;
    case FBL_RC_CONDITIONS_NOT_CORRECT:
        return QString("Conditions not correct"); break;
    case FBL_RC_REQUEST_SEQUENCE_ERROR:
        return QString("Request sequence error"); break;
    case FBL_RC_FAILURE_PREVENTS_EXEC_OF_REQUESTED_ACTION:
        return QString("Failure prevents execution of requested action"); break;
    case FBL_RC_REQUEST_OUT_OF_RANGE:
        return QString("Request out of range"); break;
    case FBL_RC_SECURITY_ACCESS_DENIED:
        return QString("Security access denied"); break;
    case FBL_RC_INVALID_KEY:
        return QString("Invalid key"); break;
    case FBL_RC_EXCEEDED_NUMBER_OF_ATTEMPTS:
        return QString("Exceeded number of attempts"); break;
    case FBL_RC_REQUIRED_TIME_DELAY_NOT_EXPIRED:
        return QString("Required time delay not expired"); break;
    case FBL_RC_UPLOAD_DOWNLOAD_NOT_ACCEPTED:
        return QString("Upload/Download not accepted"); break;
    case FBL_RC_TRANSFER_DATA_SUSPENDED:
        return QString("Transfer data suspended"); break;
    case FBL_RC_GENERAL_PROGRAMMING_FAILURE:
        return QString("General programming failure"); break;
    case FBL_RC_WRONG_BLOCK_SEQUENCE_COUNTER:
        return QString("Wrong Block Sequence Counter"); break;
    case FBL_RC_SUB_FUNC_NOT_SUPPORTED_IN_ACTIVE_SESSION:
        return QString("Sub-Function not supported in active session"); break;
    case FBL_RC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION:
        return QString("Service not supported in active session"); break;
    default:
        return QString("Negative Response Code unknown");
    }
}

/**
 * @brief Translates a given DID to the String representation of the DID according to UDS Communication documentation
 * @param DID
 * @return
 */
QString UDS::translateDID(uint16_t DID){
    switch(DID){
        case FBL_DID_APP_ID:
            return QString("Application identifier"); break;
        case FBL_DID_SYSTEM_NAME:
            return QString("System Name"); break;
        case FBL_DID_PROGRAMMING_DATE:
            return QString("Programming Date"); break;
        case FBL_DID_BL_KEY_ADDRESS:
            return QString("Key Address"); break;
        case FBL_DID_BL_KEY_GOOD_VALUE:
            return QString("Key Good Value"); break;
        case FBL_DID_CAN_BASE_MASK:
            return QString("CAN Base Mask"); break;
        case FBL_DID_CAN_ID:
            return QString("CAN ID"); break;
        case FBL_DID_BL_WRITE_START_ADD_CORE0:
            return QString("Write Start Address Core 0"); break;
        case FBL_DID_BL_WRITE_END_ADD_CORE0:
            return QString("Write End Address Core 0"); break;
        case FBL_DID_BL_WRITE_START_ADD_CORE1:
            return QString("Write Start Address Core 1"); break;
        case FBL_DID_BL_WRITE_END_ADD_CORE1:
            return QString("Write End Address Core 1"); break;
        case FBL_DID_BL_WRITE_START_ADD_CORE2:
            return QString("Write Start Address Core 2"); break;
        case FBL_DID_BL_WRITE_END_ADD_CORE2:
            return QString("Write End Address Core 2"); break;
        default:
            return QString("Data Identifier unknown");
    }
}

/**
 * @brief Reads the Data based on format of the given DID
 * @param DID To be used as format template
 * @param data Payload of the data
 * @param no_bytes Number of bytes of the data
 * @return
 */
QString UDS::readDIDData(uint16_t DID, uint8_t* data, uint32_t no_bytes){

    QString retText = "";
    switch(DID){
        case FBL_DID_APP_ID:
            return QString::fromLocal8Bit(&data[0]); break;
        case FBL_DID_SYSTEM_NAME:
            return QString::fromLocal8Bit(&data[0]); break;

        case FBL_DID_PROGRAMMING_DATE:
            if(no_bytes != 6)
                return "Wrong Programming Date format";

            if(data[0] == 0 && data[1] == 0)
                return "-";

            retText.append(QString("%1").arg(data[0], 2, 16, QLatin1Char( '0' )));          // Day
            retText.append("."+QString("%1").arg(data[1], 2, 16, QLatin1Char( '0' )));      // Month
            retText.append(".20"+QString("%1").arg(data[2], 2, 16, QLatin1Char( '0' )));    // Year

            retText.append(" "+QString("%1").arg(data[3], 2, 16, QLatin1Char( '0' )));      // Hour
            retText.append(":"+QString("%1").arg(data[4], 2, 16, QLatin1Char( '0' )));      // Minute
            retText.append(":"+QString("%1").arg(data[5], 2, 16, QLatin1Char( '0' )));      // Second

            return retText; break;

        case FBL_DID_BL_KEY_ADDRESS:
            for(int i=0; i < no_bytes; i++)
                retText.append(QString("%1").arg(data[i], 2, 16, QLatin1Char( '0' )));
            return retText; break;
        case FBL_DID_BL_KEY_GOOD_VALUE:
            for(int i=0; i < no_bytes; i++)
                retText.append(QString("%1").arg(data[i], 2, 16, QLatin1Char( '0' )));
            return retText; break;
        case FBL_DID_CAN_BASE_MASK:
            for(int i=0; i < no_bytes; i++)
                retText.append(QString("%1").arg(data[i], 2, 16, QLatin1Char( '0' )));
            return retText; break;
        case FBL_DID_CAN_ID:
            return QString("Not yet supported"); break;
        case FBL_DID_BL_WRITE_START_ADD_CORE0:
            for(int i=0; i < no_bytes; i++)
                retText.append(QString("%1").arg(data[i], 2, 16, QLatin1Char( '0' )));
            return retText; break;
        case FBL_DID_BL_WRITE_END_ADD_CORE0:
            for(int i=0; i < no_bytes; i++)
                retText.append(QString("%1").arg(data[i], 2, 16, QLatin1Char( '0' )));
            return retText; break;
        case FBL_DID_BL_WRITE_START_ADD_CORE1:
            for(int i=0; i < no_bytes; i++)
                retText.append(QString("%1").arg(data[i], 2, 16, QLatin1Char( '0' )));
            return retText; break;
        case FBL_DID_BL_WRITE_END_ADD_CORE1:
            for(int i=0; i < no_bytes; i++)
                retText.append(QString("%1").arg(data[i], 2, 16, QLatin1Char( '0' )));
            return retText; break;
        case FBL_DID_BL_WRITE_START_ADD_CORE2:
            for(int i=0; i < no_bytes; i++)
                retText.append(QString("%1").arg(data[i], 2, 16, QLatin1Char( '0' )));
            return retText; break;
        case FBL_DID_BL_WRITE_END_ADD_CORE2:
            for(int i=0; i < no_bytes; i++)
                retText.append(QString("%1").arg(data[i], 2, 16, QLatin1Char( '0' )));
            return retText; break;
        default:
            return QString("Data Identifier unknown");
    }
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

/**
 * @brief Checks if the response is available in time
 * @param waittime Time to wait for receiving the correct response
 * @return TX_OK if async, RX_NO_RESPONSE if no response in time, TX_RX_OK if response is received in time
 */
UDS::RESP UDS::checkOnResponse(uint32_t waittime){
    // No synchronization of TX to RX
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
            // Release the communication flag
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
    if(VERBOSE_UDS) qInfo("UDS: Slot - Received UDS Message to be processed");
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
