// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : testcases.cpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Testcases for UDS selftests, GUI tests and MCU tests
//============================================================================

#include <QDebug>

#include "testcases.hpp"

#include "../WINDOWS_GUI/UDS_Spec/uds_comm_spec.h"

Testcases::Testcases(){
    this->gui_id = 0x7;
    this->no_gui_id = 0x0;
    this->ecu_id = 0x1;
    this->testmode = 0;

    qInfo("Main: Create Communication Layer");
    comm = new Communication();
    comm->setCommunicationType(1); // Set to CAN
    comm->setTestMode(); // Explicitly set testMode
    comm->init(1); // Set to CAN

    qInfo("Main: Create UDS Layer and connect Communcation Layer to it");
    uds = new UDS(this->gui_id);

    //=====================================================================
    // Connect the signals and slots

    // Comm RX Signal to Testcases RX Slot
    connect(comm, SIGNAL(rxDataReceived(uint, QByteArray)), this, SLOT(rxDataReceiverSlot(uint, QByteArray)), Qt::DirectConnection);

    // UDS TX Signals to Comm TX Slots
    connect(uds, SIGNAL(setID(uint32_t)),    comm, SLOT(setIDSlot(uint32_t)));
    connect(uds, SIGNAL(txData(QByteArray)), comm, SLOT(txDataSlot(QByteArray)));
    //=====================================================================

    // GUI Console Print
    connect(uds, SIGNAL(toConsole(QString)), this, SLOT(consoleForward(QString)));
}

Testcases::~Testcases() {

}

//////////////////////////////////////////////////////////////////////////////
// Public - Testcases Interfaces
//////////////////////////////////////////////////////////////////////////////
void Testcases::setTestMode(uint8_t mode){
    // Setting Appname is found in CAN_Wrapper
    emit toConsole("\tInfo: Using AMOS TESTING as Appname (see Vector Hardware Manager)\n");

    this->testmode = mode;
}

void Testcases::startTests(){
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
// Public - Testcases RX Check
//////////////////////////////////////////////////////////////////////////////
void Testcases::messageChecker(const unsigned int id, const QByteArray &rec){

    if(rec.size() == 0){
        qInfo() << "Message from ID"<<id<<"contains no data bytes";
        return;
    }


    if(this->testmode == 0){ // Selftests
        emit toConsole("Seftest: Checking on received UDS Message with "+QString::number(rec.size())+" bytes");

        int len = 0;
        uint8_t *msg = nullptr;
        unsigned int check_id = createCommonID(FBLCAN_BASE_ADDRESS, this->gui_id, this->ecu_id) | 0x80000000; // 0x80000000 because of CAN Driver (to identify extended ID)
        unsigned int broadcast_check_id = createCommonID(FBLCAN_BASE_ADDRESS, this->gui_id, 0) | 0x80000000;

        if(rec[0] == FBL_DIAGNOSTIC_SESSION_CONTROL){
            emit toConsole(">> Received Diagnostic Session Control - Checking on content");

            // Create the relevant message
            msg = _create_diagnostic_session_control(&len, 0, FBL_DIAG_SESSION_DEFAULT); // Request Tester present from ECU
        }

        else if(rec[0] == FBL_ECU_RESET){
            emit toConsole(">> Received ECU Reset - Checking on content");

            // Create the relevant message
            msg = _create_ecu_reset(&len, 0, FBL_ECU_RESET_POWERON);
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
            msg = _create_transfer_data(&len, 0xA0090000, transfer_data, sizeof(transfer_data));
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

    else if(this->testmode == 1) { // GUI-Tests
        emit toConsole("GUI Test: Checking on received UDS Message with "+QString::number(rec.size())+" bytes");

        emit toConsole(">> NOT YET IMPLEMENTED!");

    }

    else if(this->testmode == 2){ // ECU-Tests
        emit toConsole("ECU Test: Checking on received UDS Message with "+QString::number(rec.size())+" bytes");

        emit toConsole(">> NOT YET IMPLEMENTED!");

    }
    else {
        QString log = ">> UDS Received from ID: ";
        log.append(QString("0x%1").arg(id, 8, 16, QLatin1Char( '0' )));
        log.append(" - Data=");

        for(auto i = 0; i < rec.size(); i++){
            log.append(" " + QString("0x%1").arg(rec[i], 2, 16, QLatin1Char( '0' )));
        }
        emit toConsole(log);
    }
}


//////////////////////////////////////////////////////////////////////////////
// Public - Testcases TX Creation
//////////////////////////////////////////////////////////////////////////////

void Testcases::testReqIdentification() // Sending out broadcast for tester present
{
    if(this->testmode != 0 && this->testmode != 2) // Only Seltests and ECU-Tests are transmitting data
        return;

    emit toConsole("Testcase: TX Check Request Identification for 1 ECU");

    uds->reqIdentification();
}


// Specification for Diagnostic and Communication Management
void Testcases::testDiagnosticSessionControl(){
    if(this->testmode != 0 && this->testmode != 2) // Only Seltests and ECU-Tests are transmitting data
        return;

    emit toConsole("Testcase: TX Check DiagnosticSessionControl with Default Session");
    uds->diagnosticSessionControl(this->ecu_id, FBL_DIAG_SESSION_DEFAULT);
}

void Testcases::testEcuReset(){
    if(this->testmode != 0 && this->testmode != 2) // Only Seltests and ECU-Tests are transmitting data
        return;

    //TBD: Fill Testcase for TX
    emit toConsole("Testcase: TX Check ECU Reset - Poweron");
    uds->ecuReset(this->ecu_id, FBL_ECU_RESET_POWERON);
}

void Testcases::testSecurityAccessRequestSEED(){
    if(this->testmode != 0 && this->testmode != 2) // Only Seltests and ECU-Tests are transmitting data
        return;

    //TBD: Fill Testcase for TX
    emit toConsole("Testcase: TX Check Security Access Request Seed");
    uds->securityAccessRequestSEED(this->ecu_id);
}


void Testcases::testSecurityAccessVerifyKey(){
    if(this->testmode != 0 && this->testmode != 2) // Only Seltests and ECU-Tests are transmitting data
        return;

    //TBD: Fill Testcase for TX
    emit toConsole("Testcase: TX Check Security Access Verify Key - 7 Byte Key");
    uint8_t verify_key[7] = {0x78, 0x67, 0x56, 0x45, 0x34, 0x23, 0x12};
    uds->securityAccessVerifyKey(this->ecu_id, verify_key, sizeof(verify_key));
}


void Testcases::testTesterPresent(){
    if(this->testmode != 0 && this->testmode != 2) // Only Seltests and ECU-Tests are transmitting data
        return;

    //TBD: Fill Testcase for TX
    emit toConsole("Testcase: TX Check Tester Present");
    uds->testerPresent(this->ecu_id);

}

// Specification for Data Transmission
void Testcases::testReadDataByIdentifier(){
    if(this->testmode != 0 && this->testmode != 2) // Only Seltests and ECU-Tests are transmitting data
        return;

    //TBD: Fill Testcase for TX
    emit toConsole("Testcase: TX Check Read Data By Identifier - System Name (0xF197)");
    uds->readDataByIdentifier(this->ecu_id, FBL_DID_SYSTEM_NAME);

}
void Testcases::testReadMemoryByAddress(){
    if(this->testmode != 0 && this->testmode != 2) // Only Seltests and ECU-Tests are transmitting data
        return;

    //TBD: Fill Testcase for TX
    emit toConsole("Testcase: TX Check Read Memory By Address - Address 0xA0090000, 1 Byte");
    uds->readMemoryByAddress(this->ecu_id, 0xA0090000, 1);

}

void Testcases::testWriteDataByIdentifier(){
    if(this->testmode != 0 && this->testmode != 2) // Only Seltests and ECU-Tests are transmitting data
        return;

    //TBD: Fill Testcase for TX
    emit toConsole("Testcase: TX Check Write Data By Identifier");
    uint8_t data[] = "AMOS FBL 24";
    uds->writeDataByIdentifier(this->ecu_id, FBL_DID_SYSTEM_NAME, data, sizeof(data));
}

// Specification for Upload | Download
void Testcases::testRequestDownload(){
    if(this->testmode != 0 && this->testmode != 2) // Only Seltests and ECU-Tests are transmitting data
        return;

    //TBD: Fill Testcase for TX
    emit toConsole("Testcase: TX Check Request Download - Address 0xA0090000, 127 Byte");
    uds->requestDownload(this->ecu_id, 0xA0090000, 127);
}

void Testcases::testRequestUpload(){
    if(this->testmode != 0 && this->testmode != 2) // Only Seltests and ECU-Tests are transmitting data
        return;

    //TBD: Fill Testcase for TX
    emit toConsole("Testcase: TX Check Request Upload - Address 0xA0090000, 127 Byte");
    uds->requestUpload(this->ecu_id, 0xA0090000, 127);
}

void Testcases::testTransferData(){
    if(this->testmode != 0 && this->testmode != 2) // Only Seltests and ECU-Tests are transmitting data
        return;

    //TBD: Fill Testcase for TX
    emit toConsole("Testcase: TX Check Transfer Data");
    uint8_t transfer_data[] = {0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5};
    uds->transferData(this->ecu_id, 0xA0090000, transfer_data, sizeof(transfer_data));
}

void Testcases::testRequestTransferExit(){
    if(this->testmode != 0 && this->testmode != 2) // Only Seltests and ECU-Tests are transmitting data
        return;

    //TBD: Fill Testcase for TX
    emit toConsole("Testcase: TX Check Request Transfer Exit");
    uds->requestTransferExit(this->ecu_id, 0xA0090000);
}

// Supported Common Response Codes
void Testcases::testNegativeResponse(){
    if(this->testmode != 0 && this->testmode != 2) // Only Seltests and ECU-Tests are transmitting data
        return;

    //TBD: Fill Testcase for TX
    emit toConsole("Testcase: TX Check Negative Response - Diagnostic Session Control - Conditions not Correct");
    uds->negativeResponse(this->ecu_id, FBL_DIAGNOSTIC_SESSION_CONTROL, FBL_RC_CONDITIONS_NOT_CORRECT);
}

//////////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////////

uint32_t Testcases::createCommonID(uint32_t base_id, uint8_t gui_id, uint32_t ecu_id){

	if (ecu_id > 0xFFF){
        qInfo("ID is not supported");
		return 0;
	}

	uint32_t send_id = (uint32_t)(base_id | gui_id);
	send_id |= (ecu_id<<4); // Combine ECU ID with send ID including GUI ID (see spec)

	return send_id;
}


uint8_t Testcases::checkEqual(unsigned int recid, const QByteArray &rec, unsigned int checkid, QByteArray &check){

    QString s;
    QTextStream out(&s);

    uint8_t result = 1;

    // Checking on IDs
    if(recid != checkid){
        out << ">> Testcase - ERROR - ID is different. Rec=" << QString("0x%1").arg(uint32_t(recid), 8, 16, QLatin1Char( '0' )) << "!= Check=" <<QString("0x%1").arg(uint32_t(checkid), 8, 16, QLatin1Char( '0' )) << "\n";
        result = 0;
    }
    else{
        out << ">> Testcase - PASSED - ID is equal. Rec=" << QString("0x%1").arg(uint32_t(recid), 8, 16, QLatin1Char( '0' )) << "== Check=" <<QString("0x%1").arg(uint32_t(checkid), 8, 16, QLatin1Char( '0' )) << "\n";

    }

    // Extract messages

    if(rec.size() != check.size()){
        out << ">> Testcase - ERROR - Length is different. Rec=" << rec.size() << "!= Check=" <<check.size()<< "\n";
        emit toConsole(*out.string());
        result = 0;
    }
    else{
        out << ">> Testcase - PASSED - Length is equal. Rec=" << rec.size() << "== Check=" <<check.size()<< "\n";
    }

    uint8_t error = 0;
    for(auto i = 0; i < rec.size(); i++){
        if(rec[i] != check[i]){
            out << ">> Testcase - ERROR - Content is different at index " << i<<", Received: "<<QString("0x%1").arg(uint8_t(rec[i]), 2, 16, QLatin1Char( '0' ))<<" != Check: "<< QString("0x%1").arg(uint8_t(check[i]), 2, 16, QLatin1Char( '0' ))<< "\n";
            error=1;
        }
        else {
            out << ">> Testcase - PASSED - Content is equal at index " << i<<", Received: "<<QString("0x%1").arg(uint8_t(rec[i]), 2, 16, QLatin1Char( '0' ))<<" == Check: "<< QString("0x%1").arg(uint8_t(check[i]), 2, 16, QLatin1Char( '0' ))<< "\n";
        }
    }
    if(error){
        emit toConsole(*out.string() + "\n");
        result = 0;
    }

    if(result == 1)
        out << ">> Testcase - PASSED COMPLETELY!\n";

    emit toConsole(*out.string());
    return result == 1;
}

//============================================================================
// Slots
//============================================================================

void Testcases::rxDataReceiverSlot(const unsigned int id, const QByteArray &ba){
    qInfo(">> Testcases: Received UDS Message to be processed for testcases");
    this->messageChecker(id, ba);
}

void Testcases::consoleForward(const QString &console){
    qInfo("Testcases: Sending Signal to forward message");
    emit toConsole(console);
}
