// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : testcase.cpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Base class for specific Testcase
//============================================================================

#include <QThread>

#include "testcase.hpp"

Testcase::Testcase(uint8_t gui_id){
    this->gui_id = gui_id;
    this->no_gui_id = 0x0;

    this->ecu_id = 0x1;

    qInfo("Testcase: Create Communication Layer");
    comm = new Communication();
    comm->setCommunicationType(Communication::CAN_DRIVER); // Set to CAN
    comm->setTestMode(); // Explicitly set testMode
    comm->init(Communication::CAN_DRIVER); // Set to CAN

    qInfo("Testcase: Create UDS Layer and connect Communcation Layer to it");
    uds = new UDS(this->gui_id);
    uds->setSyncMode(false);

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

Testcase::~Testcase(){
    // Disconnect everything
    disconnect(uds, nullptr, nullptr, nullptr);
    disconnect(comm, nullptr, nullptr, nullptr);

    delete uds;
    delete comm;

    qInfo() << "Testcase: Destructor of Base class is finished";
}

//============================================================================
// Public
//============================================================================

void Testcase::messageChecker(const unsigned int id, const QByteArray &rec){
    emit toConsole("To be implemented in inheriting class");
}

void Testcase::startTests(){
    emit toConsole("To be implemented in inheriting class");
}

//============================================================================
// Protected
//============================================================================

uint32_t Testcase::createCommonID(uint32_t base_id, uint8_t gui_id, uint32_t ecu_id){

    if (ecu_id > 0xFFF){
        qInfo("ID is not supported");
        return 0;
    }

    uint32_t send_id = (uint32_t)(base_id | gui_id);
    send_id |= (ecu_id<<4); // Combine ECU ID with send ID including GUI ID (see spec)

    return send_id;
}


uint8_t Testcase::checkEqual(unsigned int recid, const QByteArray &rec, unsigned int checkid, QByteArray &check){

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

void Testcase::rxDataReceiverSlot(const unsigned int id, const QByteArray &ba){
    qInfo(">> Testcases: Received UDS Message to be processed for testcases");
    this->messageChecker(id, ba);
}

void Testcase::consoleForward(const QString &console){
    emit toConsole(console);
}

