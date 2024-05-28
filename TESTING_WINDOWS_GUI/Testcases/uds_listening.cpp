// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : uds_listening.cpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Class for UDS Listening only
//============================================================================

#include "uds_listening.hpp"

UDS_Listening::UDS_Listening(uint8_t gui_id) : Testcase(gui_id){

}

UDS_Listening::~UDS_Listening(){

}

//////////////////////////////////////////////////////////////////////////////
// Public - RX
//////////////////////////////////////////////////////////////////////////////

void UDS_Listening::messageChecker(const unsigned int id, const QByteArray &rec){

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

void UDS_Listening::startTests(){
    emit toConsole("Listening for UDS Message only. No TX");

}
