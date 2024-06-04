// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>

//============================================================================
// Name        : ecu_test.cpp
// Author      : Michael Bauer, Wiktor Pilarczyk
// Version     : 0.2
// Copyright   : MIT
// Description : Class for ECU testcases
//============================================================================

#include "ecu_test.hpp"

#include "../../WINDOWS_GUI/UDS_Spec/uds_comm_spec.h"

#include <QDateTime>

ECU_Test::ECU_Test(uint8_t gui_id) : Testcase(gui_id){
    writing_test = true; // Change if writing test should be activated
}

ECU_Test::~ECU_Test(){

}

//////////////////////////////////////////////////////////////////////////////
// Public - RX
//////////////////////////////////////////////////////////////////////////////

void ECU_Test::messageChecker(const unsigned int id, const QByteArray &rec){

    if(rec.size() == 0){
        qInfo() << "Message from ID"<<id<<"contains no data bytes";
        return;
    }

    int len = 0;
    uint8_t *msg = nullptr;
    unsigned int own_id = createCommonID(FBLCAN_BASE_ADDRESS, this->gui_id, this->ecu_id); // 0x80000000 because of CAN Driver (to identify extended ID)
    unsigned int ecu_id = createCommonID(FBLCAN_BASE_ADDRESS, 0           , this->ecu_id);
    unsigned int broadcast_check_id = createCommonID(FBLCAN_BASE_ADDRESS, this->gui_id, 0);

    if(id == own_id || id == broadcast_check_id){
        //emit toConsole("ECU Test: Ignoring UDS Message with Testing GUI ID");
        return;
    }
    emit toConsole("ECU Test: Checking on received UDS Message with "+QString::number(rec.size())+" bytes");

    if(rec[0] == FBL_NEGATIVE_RESPONSE){

        QString nrc = uds->translateNegResp(rec[2]);
        QString log = ">> Received Negative Response for Service " + QString("0x%1").arg(uint8_t(rec[1]), 2, 16, QLatin1Char( '0' )) + " - NRC: "+nrc +" ("+ QString::number(rec[2])+")\n";
        emit toConsole(log);
        return;
    }

    uint8_t sid = rec[0] - FBL_SID_ACK;

    if(sid == FBL_DIAGNOSTIC_SESSION_CONTROL){
        emit toConsole(">> Received Diagnostic Session Control - Checking on content");

        // Create the relevant message
        if(rec[1] == FBL_DIAG_SESSION_DEFAULT)
            msg = _create_diagnostic_session_control(&len, 1, FBL_DIAG_SESSION_DEFAULT);
        else if (rec[1] == FBL_DIAG_SESSION_PROGRAMMING)
            msg = _create_diagnostic_session_control(&len, 1, FBL_DIAG_SESSION_PROGRAMMING);
    }

    else if(sid == FBL_ECU_RESET){
        emit toConsole(">> Received ECU Reset - Checking on content");

        // Create the relevant message
        if(rec[1] == FBL_ECU_RESET_HARD)
            msg = _create_ecu_reset(&len, 1, FBL_ECU_RESET_HARD);
        else if(rec[1] == FBL_ECU_RESET_SOFT)
            msg = _create_ecu_reset(&len, 1, FBL_ECU_RESET_SOFT);
    }

    else if(sid == FBL_TESTER_PRESENT){
        emit toConsole(">> Received Tester Present - Checking on content");
        msg = _create_tester_present(&len, 1, FBL_TESTER_PRES_WITH_RESPONSE);
    }

    else if(sid == FBL_READ_DATA_BY_IDENTIFIER){
        emit toConsole(">> Received Read Data by Identifier - Checking on content");

        // Create the relevant message
        uint16_t did = 0;
        uint8_t max_idx = 2;
        for(int i = max_idx; i >= 1; i--){
            did |= ((uint8_t)(rec[i]) << (8*(max_idx-i)));
        }
        emit toConsole(">> Received Read Data by Identifiert for DID " + QString("0x%1").arg(did, 2, 16, QLatin1Char( '0' )));
        if(did == FBL_DID_SYSTEM_NAME){
            if(writing_test){
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_SYSTEM_NAME, did_system_name, did_system_name_len);
            }
            else{
                uint8_t check_data[] = "AMOS FBL 24";
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_SYSTEM_NAME, check_data, sizeof(check_data));
            }

        }

        else if(did == FBL_DID_PROGRAMMING_DATE){
            if(writing_test){
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_PROGRAMMING_DATE, did_programming_date, sizeof(did_programming_date));
            }
            else{
                uint8_t check_data[] = {0x30, 0x05, 0x24};
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_PROGRAMMING_DATE, check_data, sizeof(check_data));
            }

        }

        else if(did == FBL_DID_BL_KEY_ADDRESS){
            if(writing_test){
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_BL_KEY_ADDRESS, did_bl_key_address, sizeof(did_bl_key_address));
            }
            else{
                uint8_t check_data[] = {0xA0, 0x4F, 0x80, 0x09};
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_BL_KEY_ADDRESS, check_data, sizeof(check_data));
            }

        }

        else if(did == FBL_DID_BL_KEY_GOOD_VALUE){
            if(writing_test){
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_BL_KEY_GOOD_VALUE, did_bl_key_good_value, sizeof(did_bl_key_good_value));
            }
            else{
                uint8_t check_data[] = {0x93, 0x86, 0xC3, 0xA5};
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_BL_KEY_GOOD_VALUE, check_data, sizeof(check_data));
            }

        }

        else if(did == FBL_DID_CAN_BASE_MASK){
            if(writing_test){
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_CAN_BASE_MASK, did_can_base_mask, sizeof(did_can_base_mask));
            }
            else{
                uint8_t check_data[] = {0x0F, 0x24};
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_CAN_BASE_MASK, check_data, sizeof(check_data));
            }

        }

        else if(did == FBL_DID_CAN_ID){
            if(writing_test){
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_CAN_ID, did_can_id, sizeof(did_can_id));
            }
            else{
                uint8_t check_data[] = {0x00, 0x01};
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_CAN_ID, check_data, sizeof(check_data));
            }

        }

        else if(did == FBL_DID_BL_WRITE_START_ADD_CORE0){
            if(writing_test){
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_BL_WRITE_START_ADD_CORE0, did_bl_write_start_add_core0, sizeof(did_bl_write_start_add_core0));
            }
            else{
                uint8_t check_data[] = {0xA0, 0x09, 0x00, 0x00};
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_BL_WRITE_START_ADD_CORE0, check_data, sizeof(check_data));
            }

        }

        else if(did == FBL_DID_BL_WRITE_END_ADD_CORE0){
            if(writing_test){
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_BL_WRITE_END_ADD_CORE0, did_bl_write_end_add_core0, sizeof(did_bl_write_end_add_core0));
            }
            else{
                uint8_t check_data[] = {0xA0, 0x1F, 0xFF, 0xFF};
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_BL_WRITE_END_ADD_CORE0, check_data, sizeof(check_data));
            }

        }

        else if(did == FBL_DID_BL_WRITE_START_ADD_CORE1){
            if(writing_test){
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_BL_WRITE_START_ADD_CORE1, did_bl_write_start_add_core1, sizeof(did_bl_write_start_add_core1));
            }
            else{
                uint8_t check_data[] = {0xA0, 0x30, 0x40, 0x00};
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_BL_WRITE_START_ADD_CORE1, check_data, sizeof(check_data));
            }

        }

        else if(did == FBL_DID_BL_WRITE_END_ADD_CORE1){
            if(writing_test){
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_BL_WRITE_END_ADD_CORE1, did_bl_write_end_add_core1, sizeof(did_bl_write_end_add_core1));
            }
            else{
                uint8_t check_data[] = {0xA0, 0x4F, 0xF7, 0xFF};
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_BL_WRITE_END_ADD_CORE1, check_data, sizeof(check_data));
            }

        }

        else if(did == FBL_DID_BL_WRITE_START_ADD_CORE2){
            if(writing_test){
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_BL_WRITE_START_ADD_CORE2, did_bl_write_start_add_core2, sizeof(did_bl_write_start_add_core2));
            }
            else{
                uint8_t check_data[] = {0xFF, 0xFF, 0xFF, 0xFF};
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_BL_WRITE_START_ADD_CORE2, check_data, sizeof(check_data));
            }

        }

        else if(did == FBL_DID_BL_WRITE_END_ADD_CORE2){
            if(writing_test){
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_BL_WRITE_END_ADD_CORE2, did_bl_write_end_add_core2, sizeof(did_bl_write_end_add_core2));
            }
            else{
                uint8_t check_data[] = {0xFF, 0xFF, 0xFF, 0xFF};
                msg = _create_read_data_by_ident(&len, 1, FBL_DID_BL_WRITE_END_ADD_CORE2, check_data, sizeof(check_data));
            }
        }
    }

    else if(sid == FBL_WRITE_DATA_BY_IDENTIFIER){
        emit toConsole(">> Received Write Data By Identifier - Checking on content");

        // Create the relevant message

        // Create the relevant message
        uint16_t did = 0;
        uint8_t max_idx = 2;
        for(int i = max_idx; i >= 1; i--){
            did |= ((uint8_t)(rec[i]) << (8*(max_idx-i)));
        }
        msg = _create_write_data_by_ident(&len, 1, did, 0, 0);
    }

    if(len > 0){
        // Wrap data into QByteArray
        QByteArray check;
        check.resize(len);
        for(int i=0; i < check.size(); i++)
            check[i] = msg[i];
        // Free the allocated memory of msg
        free(msg);

        this->checkEqual(id, rec, ecu_id, check);
    }

    else {
        // Listening only
        QString log = ">> Undefined Response: UDS Received from ID: ";
        log.append(QString("0x%1").arg(id, 8, 16, QLatin1Char( '0' )));
        log.append(" - Data=");

        for(auto i = 0; i < rec.size(); i++){
            log.append(" " + QString("0x%1").arg(uint8_t(rec[i]), 2, 16, QLatin1Char( '0' )));
        }
        emit toConsole(log);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Public - TX
//////////////////////////////////////////////////////////////////////////////

void ECU_Test::startTests(){
    emit toConsole("Start of TX Section");

    // Sending out broadcast for tester present
    testReqIdentification();

    // Specification for Diagnostic and Communication Management
    testEcuReset();
    testDiagnosticSessionControl();
    testTesterPresent();

    // Specification for Data Transmission
    if(writing_test) // Test Writing - Programming session already activated
        testWriteDataByIdentifier();
    testReadDataByIdentifier();

    emit toConsole("End of TX Section\n");

    emit toConsole("Start of RX Section");
    // RX will come in asynchronous
}

//////////////////////////////////////////////////////////////////////////////
// Private - Testcases TX Creation
//////////////////////////////////////////////////////////////////////////////

void ECU_Test::testReqIdentification() // Sending out broadcast for tester present
{
    emit toConsole("ECU Test: TX Check Request Identification for 1 ECU");
    uds->reqIdentification();
}


// Specification for Diagnostic and Communication Management
void ECU_Test::testDiagnosticSessionControl()
{
    emit toConsole("ECU Test: TX Check DiagnosticSessionControl with Default Session");
    uds->diagnosticSessionControl(this->ecu_id, FBL_DIAG_SESSION_DEFAULT);

    emit toConsole("ECU Test: TX Check DiagnosticSessionControl with Programming Session");
    uds->diagnosticSessionControl(this->ecu_id, FBL_DIAG_SESSION_PROGRAMMING);
}

void ECU_Test::testEcuReset()
{
    emit toConsole("ECU Test: TX Check ECU Reset - HARD");
    uds->ecuReset(this->ecu_id, FBL_ECU_RESET_HARD);

    qint64 start = QDateTime::currentSecsSinceEpoch();
    while(QDateTime::currentSecsSinceEpoch() - start <= 3){}

    emit toConsole("ECU Test: TX Check ECU Reset - SOFT");
    uds->ecuReset(this->ecu_id, FBL_ECU_RESET_SOFT);

    start = QDateTime::currentSecsSinceEpoch();
    while(QDateTime::currentSecsSinceEpoch() - start <= 3){}
}


void ECU_Test::testTesterPresent()
{
    emit toConsole("ECU Test: TX Check Tester Present");
    uds->testerPresent(this->ecu_id);

}

// Specification for Data Transmission
void ECU_Test::testReadDataByIdentifier()
{
    emit toConsole("ECU Test: TX Check Read Data By Identifier - System Name (0xF197)");
    uds->readDataByIdentifier(this->ecu_id, FBL_DID_SYSTEM_NAME);

    emit toConsole("ECU Test: TX Check Read Data By Identifier - Programming Date (0xF199)");
    uds->readDataByIdentifier(this->ecu_id, FBL_DID_PROGRAMMING_DATE);

    emit toConsole("ECU Test: TX Check Read Data By Identifier - Key Address (0xFD00)");
    uds->readDataByIdentifier(this->ecu_id, FBL_DID_BL_KEY_ADDRESS);

    emit toConsole("ECU Test: TX Check Read Data By Identifier - Key Good Value (0xFD01)");
    uds->readDataByIdentifier(this->ecu_id, FBL_DID_BL_KEY_GOOD_VALUE);

    emit toConsole("ECU Test: TX Check Read Data By Identifier - CAN Base Mask (0xFD02)");
    uds->readDataByIdentifier(this->ecu_id, FBL_DID_CAN_BASE_MASK);

    emit toConsole("ECU Test: TX Check Read Data By Identifier - CAN ID (0xFD03)");
    uds->readDataByIdentifier(this->ecu_id, FBL_DID_CAN_ID);

    emit toConsole("ECU Test: TX Check Read Data By Identifier - Write Start Address Core 0 (0xFD10)");
    uds->readDataByIdentifier(this->ecu_id, FBL_DID_BL_WRITE_START_ADD_CORE0);

    emit toConsole("ECU Test: TX Check Read Data By Identifier - Write End Address Core 0 (0xFD11)");
    uds->readDataByIdentifier(this->ecu_id, FBL_DID_BL_WRITE_END_ADD_CORE0);

    emit toConsole("ECU Test: TX Check Read Data By Identifier - Write Start Address Core 1 (0xFD12)");
    uds->readDataByIdentifier(this->ecu_id, FBL_DID_BL_WRITE_START_ADD_CORE1);

    emit toConsole("ECU Test: TX Check Read Data By Identifier - Write End Address Core 1 (0xFD13)");
    uds->readDataByIdentifier(this->ecu_id, FBL_DID_BL_WRITE_END_ADD_CORE1);

    emit toConsole("ECU Test: TX Check Read Data By Identifier - Write Start Address Core 2 (0xFD14)");
    uds->readDataByIdentifier(this->ecu_id, FBL_DID_BL_WRITE_START_ADD_CORE2);

    emit toConsole("ECU Test: TX Check Read Data By Identifier - Write End Address Core 2 (0xFD15)");
    uds->readDataByIdentifier(this->ecu_id, FBL_DID_BL_WRITE_END_ADD_CORE2);
}

void ECU_Test::testWriteDataByIdentifier()
{
    emit toConsole("ECU Test: TX Check Write Data By Identifier");
    uds->writeDataByIdentifier(this->ecu_id, FBL_DID_SYSTEM_NAME, did_system_name, sizeof(did_system_name));

    emit toConsole("ECU Test: TX Check Write Data By Identifier - Programming Date (0xF199)");
    uds->writeDataByIdentifier(this->ecu_id, FBL_DID_PROGRAMMING_DATE, did_programming_date, sizeof(did_programming_date));

    emit toConsole("ECU Test: TX Check Write Data By Identifier - Key Address (0xFD00)");
    uds->writeDataByIdentifier(this->ecu_id, FBL_DID_BL_KEY_ADDRESS, did_bl_key_address, sizeof(did_bl_key_address));

    emit toConsole("ECU Test: TX Check Write Data By Identifier - Key Good Value (0xFD01)");
    uds->writeDataByIdentifier(this->ecu_id, FBL_DID_BL_KEY_GOOD_VALUE, did_bl_key_good_value, sizeof(did_bl_key_good_value));

    emit toConsole("ECU Test: TX Check Write Data By Identifier - CAN Base Mask (0xFD02)");
    uds->writeDataByIdentifier(this->ecu_id, FBL_DID_CAN_BASE_MASK, did_can_base_mask, sizeof(did_can_base_mask));

    emit toConsole("ECU Test: TX Check Write Data By Identifier - CAN ID (0xFD03)");
    uds->writeDataByIdentifier(this->ecu_id, FBL_DID_CAN_ID, did_can_id, sizeof(did_can_id));

    emit toConsole("ECU Test: TX Check Write Data By Identifier - Write Start Address Core 0 (0xFD10)");
    uds->writeDataByIdentifier(this->ecu_id, FBL_DID_BL_WRITE_START_ADD_CORE0, did_bl_write_start_add_core0, sizeof(did_bl_write_start_add_core0));

    emit toConsole("ECU Test: TX Check Write Data By Identifier - Write End Address Core 0 (0xFD11)");
    uds->writeDataByIdentifier(this->ecu_id, FBL_DID_BL_WRITE_END_ADD_CORE0, did_bl_write_end_add_core0, sizeof(did_bl_write_end_add_core0));

    emit toConsole("ECU Test: TX Check Write Data By Identifier - Write Start Address Core 1 (0xFD12)");
    uds->writeDataByIdentifier(this->ecu_id, FBL_DID_BL_WRITE_START_ADD_CORE1, did_bl_write_start_add_core1, sizeof(did_bl_write_start_add_core1));

    emit toConsole("ECU Test: TX Check Write Data By Identifier - Write End Address Core 1 (0xFD13)");
    uds->writeDataByIdentifier(this->ecu_id, FBL_DID_BL_WRITE_END_ADD_CORE1, did_bl_write_end_add_core1, sizeof(did_bl_write_end_add_core1));

    emit toConsole("ECU Test: TX Check Write Data By Identifier - Write Start Address Core 2 (0xFD14)");
    uds->writeDataByIdentifier(this->ecu_id, FBL_DID_BL_WRITE_START_ADD_CORE2, did_bl_write_start_add_core2, sizeof(did_bl_write_start_add_core2));

    emit toConsole("ECU Test: TX Check Write Data By Identifier - Write End Address Core 2 (0xFD15)");
    uds->writeDataByIdentifier(this->ecu_id, FBL_DID_BL_WRITE_END_ADD_CORE2, did_bl_write_end_add_core2, sizeof(did_bl_write_end_add_core2));
}


