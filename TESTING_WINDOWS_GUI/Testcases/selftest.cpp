// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>

//============================================================================
// Name        : selftest.cpp
// Author      : Michael Bauer, Wiktor Pilarczyk
// Version     : 0.2
// Copyright   : MIT
// Description : Class for Selftest testcases
//============================================================================

#include "selftest.hpp"

#include "../../WINDOWS_GUI/UDS_Spec/uds_comm_spec.h"

Selftest::Selftest(uint8_t gui_id) : Testcase(gui_id){

}

Selftest::~Selftest(){

}

//////////////////////////////////////////////////////////////////////////////
// Public - RX
//////////////////////////////////////////////////////////////////////////////

void Selftest::messageChecker(const unsigned int id, const QByteArray &rec){

    if(rec.size() == 0){
        qInfo() << "Message from ID"<<id<<"contains no data bytes";
        return;
    }

    emit toConsole("Seftest: Checking on received UDS Message with "+QString::number(rec.size())+" bytes");

    int len = 0;
    uint8_t *msg = nullptr;
    unsigned int check_id = createCommonID(FBLCAN_BASE_ADDRESS, this->gui_id, this->ecu_id);
    unsigned int broadcast_check_id = createCommonID(FBLCAN_BASE_ADDRESS, this->gui_id, 0);

    if(rec[0] == FBL_DIAGNOSTIC_SESSION_CONTROL){
        emit toConsole(">> Received Diagnostic Session Control - Checking on content");

        // Create the relevant message
        msg = _create_diagnostic_session_control(&len, 0, FBL_DIAG_SESSION_DEFAULT); // Request Tester present from ECU
    }

    else if(rec[0] == FBL_ECU_RESET){
        emit toConsole(">> Received ECU Reset - Checking on content");

        // Create the relevant message
        msg = _create_ecu_reset(&len, 0, FBL_ECU_RESET_HARD);
    }

    else if(rec[0] == FBL_SECURITY_ACCESS){


        // Create the relevant message

        if(rec[1] == FBL_SEC_ACCESS_SEED){
            emit toConsole(">> Received Security Access SEED - Checking on content");
            msg = _create_security_access(&len, 0, FBL_SEC_ACCESS_SEED, 0, 0);
        } else if (rec[1] == FBL_SEC_ACCESS_VERIFY_KEY){
            emit toConsole(">> Received Security Access Verify Key - Checking on content");
            uint8_t verify_key[7] = {0x78, 0x67, 0x56, 0x45, 0x34, 0x23, 0x12};
            msg = _create_security_access(&len, 0, FBL_SEC_ACCESS_VERIFY_KEY, verify_key, sizeof(verify_key));
        }
    }

    else if(rec[0] == FBL_TESTER_PRESENT){
        // Create the relevant message
        if (id == broadcast_check_id){ // Received broadcast!
            emit toConsole(">> Received Tester Present Broadcast- Checking on content");
            check_id = broadcast_check_id;
            msg = _create_tester_present(&len, 0, FBL_TESTER_PRES_WITH_RESPONSE);
        }

        else { // Normal Tester Present from GUI
            emit toConsole(">> Received Tester Present - Checking on content");
            msg = _create_tester_present(&len, 0, FBL_TESTER_PRES_WITHOUT_RESPONSE);
        }
    }

    else if(rec[0] == FBL_READ_DATA_BY_IDENTIFIER){
        emit toConsole(">> Received Read Data by Identifier - Checking on content");

        // Create the relevant message
        msg = _create_read_data_by_ident(&len, 0, FBL_DID_SYSTEM_NAME, 0, 0);
    }

    else if(rec[0] == FBL_READ_MEMORY_BY_ADDRESS){
        emit toConsole(">> Received Read Memory By Address - Checking on content");

        // Create the relevant message
        msg = _create_read_memory_by_address(&len, 0, 0xA0090000, 1, 0, 0);
    }

    else if(rec[0] == FBL_WRITE_DATA_BY_IDENTIFIER){
        emit toConsole(">> Received Write Data By Identifier - Checking on content");

        // Create the relevant message
        uint8_t data[] = "AMOS FBL 24";
        msg = _create_write_data_by_ident(&len, 0, FBL_DID_SYSTEM_NAME, data, sizeof(data));
    }

    else if(rec[0] == FBL_REQUEST_DOWNLOAD){
        emit toConsole(">> Received Request Download - Checking on content");

        // Create the relevant message
        msg = _create_request_download(&len, 0, 0xA0090000, 127);
    }

    else if(rec[0] == FBL_REQUEST_UPLOAD){
        emit toConsole(">> Received Request Upload - Checking on content");

        // Create the relevant message
        msg = _create_request_upload(&len, 0, 0xA0090000, 127);
    }

    else if(rec[0] == FBL_TRANSFER_DATA){
        emit toConsole(">> Received Transfer Data - Checking on content");

        // Create the relevant message
        uint8_t transfer_data[] = {0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5};
        msg = _create_transfer_data(&len, 0xA0090000, 0, transfer_data, sizeof(transfer_data));
    }

    else if(rec[0] == FBL_REQUEST_TRANSFER_EXIT){
        emit toConsole(">> Received Request Transfer Exit - Checking on content");

        // Create the relevant message
        msg = _create_request_transfer_exit(&len, 0, 0xA0090000);
    }

    else if(rec[0] == FBL_NEGATIVE_RESPONSE){
        emit toConsole(">> Received Negative Response - Checking on content");

        // Create the relevant message
        msg = _create_neg_response(&len, FBL_DIAGNOSTIC_SESSION_CONTROL, FBL_RC_CONDITIONS_NOT_CORRECT);
    }

    if(len > 0){
        // Wrap data into QByteArray
        QByteArray check;
        check.resize(len);
        for(int i=0; i < check.size(); i++)
            check[i] = msg[i];
        // Free the allocated memory of msg
        free(msg);

        this->checkEqual(id, rec, check_id, check);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Public - TX
//////////////////////////////////////////////////////////////////////////////

void Selftest::startTests(){
    emit toConsole("Start of TX Section");

    // Sending out broadcast for tester present
    testReqIdentification();

    // Specification for Diagnostic and Communication Management
    testDiagnosticSessionControl();
    testEcuReset();
    testSecurityAccessRequestSEED();
    testSecurityAccessVerifyKey();
    testTesterPresent();

    // Specification for Data Transmission
    testReadDataByIdentifier();
    testReadMemoryByAddress();
    testWriteDataByIdentifier();

    // Specification for Upload | Download
    testRequestDownload();
    testRequestUpload();
    testTransferData();
    testRequestTransferExit();

    // Supported Common Response Codes
    testNegativeResponse();
    emit toConsole("End of TX Section\n");

    emit toConsole("Start of RX Section");
    // RX will come in asynchronous
}

//////////////////////////////////////////////////////////////////////////////
// Private - Testcases TX Creation
//////////////////////////////////////////////////////////////////////////////

void Selftest::testReqIdentification() // Sending out broadcast for tester present
{
    emit toConsole("Selftest: TX Check Request Identification for 1 ECU");
    uds->reqIdentification();
}


// Specification for Diagnostic and Communication Management
void Selftest::testDiagnosticSessionControl()
{
    emit toConsole("Selftest: TX Check DiagnosticSessionControl with Default Session");
    uds->diagnosticSessionControl(this->ecu_id, FBL_DIAG_SESSION_DEFAULT);
}

void Selftest::testEcuReset()
{
    emit toConsole("Selftest: TX Check ECU Reset - HARD");
    uds->ecuReset(this->ecu_id, FBL_ECU_RESET_HARD);
}

void Selftest::testSecurityAccessRequestSEED()
{
    emit toConsole("Selftest: TX Check Security Access Request Seed");
    uds->securityAccessRequestSEED(this->ecu_id);
}


void Selftest::testSecurityAccessVerifyKey()
{
    emit toConsole("Selftest: TX Check Security Access Verify Key - 7 Byte Key");
    uint8_t verify_key[7] = {0x78, 0x67, 0x56, 0x45, 0x34, 0x23, 0x12};
    uds->securityAccessVerifyKey(this->ecu_id, verify_key, sizeof(verify_key));
}


void Selftest::testTesterPresent()
{
    emit toConsole("Selftest: TX Check Tester Present");
    uds->testerPresent(this->ecu_id);

}

// Specification for Data Transmission
void Selftest::testReadDataByIdentifier()
{
    emit toConsole("Selftest: TX Check Read Data By Identifier - System Name (0xF197)");
    uds->readDataByIdentifier(this->ecu_id, FBL_DID_SYSTEM_NAME);

}
void Selftest::testReadMemoryByAddress()
{
    emit toConsole("Selftest: TX Check Read Memory By Address - Address 0xA0090000, 1 Byte");
    uds->readMemoryByAddress(this->ecu_id, 0xA0090000, 1);

}

void Selftest::testWriteDataByIdentifier()
{
    emit toConsole("Selftest: TX Check Write Data By Identifier");
    uint8_t data[] = "AMOS FBL 24";
    uds->writeDataByIdentifier(this->ecu_id, FBL_DID_SYSTEM_NAME, data, sizeof(data));
}

// Specification for Upload | Download
void Selftest::testRequestDownload()
{
    emit toConsole("Selftest: TX Check Request Download - Address 0xA0090000, 127 Byte");
    uds->requestDownload(this->ecu_id, 0xA0090000, 127);
}

void Selftest::testRequestUpload()
{
    emit toConsole("Selftest: TX Check Request Upload - Address 0xA0090000, 127 Byte");
    uds->requestUpload(this->ecu_id, 0xA0090000, 127);
}

void Selftest::testTransferData()
{
    emit toConsole("Selftest: TX Check Transfer Data");
    uint8_t transfer_data[] = {0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5};
    uds->transferData(this->ecu_id, 0xA0090000, transfer_data, sizeof(transfer_data));
}

void Selftest::testRequestTransferExit()
{
    emit toConsole("Selftest: TX Check Request Transfer Exit");
    uds->requestTransferExit(this->ecu_id, 0xA0090000);
}

// Supported Common Response Codes
void Selftest::testNegativeResponse()
{
    emit toConsole("Selftest: TX Check Negative Response - Diagnostic Session Control - Conditions not Correct");
    uds->negativeResponse(this->ecu_id, FBL_DIAGNOSTIC_SESSION_CONTROL, FBL_RC_CONDITIONS_NOT_CORRECT);
}
