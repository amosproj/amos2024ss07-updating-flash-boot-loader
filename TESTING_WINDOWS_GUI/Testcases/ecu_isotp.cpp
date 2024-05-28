// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : uds_listening.cpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Class for UDS Listening only
//============================================================================

#include "ecu_isotp.hpp"

#include "../../WINDOWS_GUI/UDS_Spec/uds_comm_spec.h"

ECU_ISOTP::ECU_ISOTP(uint8_t gui_id) : Testcase(gui_id){

}

ECU_ISOTP::~ECU_ISOTP(){

}

//////////////////////////////////////////////////////////////////////////////
// Public - RX
//////////////////////////////////////////////////////////////////////////////

void ECU_ISOTP::messageChecker(const unsigned int id, const QByteArray &rec){

    if(rec.size() == 0){
        qInfo() << "Message from ID"<<id<<"contains no data bytes";
        return;
    }

    // Listening only
    QString log = ">> UDS Received from ID: ";
    log.append(QString("0x%1").arg(id, 8, 16, QLatin1Char( '0' )));
    log.append(" - Data=");

    for(auto i = 0; i < rec.size(); i++){
        log.append(" " + QString("0x%1").arg(uint8_t(rec[i]), 2, 16, QLatin1Char( '0' )));
    }
    emit toConsole(log);
}

//////////////////////////////////////////////////////////////////////////////
// Public - TX
//////////////////////////////////////////////////////////////////////////////

void ECU_ISOTP::startTests(){
    emit toConsole("Start of TX Section");

    emit toConsole("Sending Single Frame UDS Message");
    testTesterPresent();

    emit toConsole("Sending Multi Frame UDS Message");
    testWriteDataByIdentifier();

    emit toConsole("End of TX Section\n");
}

//////////////////////////////////////////////////////////////////////////////
// Private - Testcases TX Creation
//////////////////////////////////////////////////////////////////////////////


void ECU_ISOTP::testTesterPresent()
{
    emit toConsole("MCU ISO TP: TX Check Tester Present");
    uds->testerPresent(this->ecu_id);

}

void ECU_ISOTP::testWriteDataByIdentifier()
{
    emit toConsole("MCU ISO TP: TX Check Write Data By Identifier");
    uint8_t data[] = "AMOS FBL 24";
    uds->writeDataByIdentifier(this->ecu_id, FBL_DID_SYSTEM_NAME, data, sizeof(data));
}

