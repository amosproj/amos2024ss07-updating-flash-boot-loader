// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : testcasecontroller.cpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Testcase Controller for different UDS tests
//============================================================================

#include <QDebug>
#include "testcasecontroller.hpp"

Testcasecontroller::Testcasecontroller(){
    this->testcase = SELFTEST;

    // Initialize different testcases with nullptr
    selftest = nullptr;
    uds_listening = nullptr;
    mcu_isotp = nullptr;
}

Testcasecontroller::~Testcasecontroller() {
    cleanUpTestcases();
}

//////////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////////
void Testcasecontroller::setTestMode(Testcasecontroller::TESTMODES mode){
    // Setting Appname is found in CAN_Wrapper
    emit toConsole("\tInfo: Using AMOS TESTING as Appname (see Vector Hardware Manager)\n");

    // Delete all old testcases
    cleanUpTestcases();

    // Create the testcase
    if(mode == SELFTEST){
        // Create the instance
        this->selftest = new Selftest(0x7);

        // GUI Console Print
        connect(selftest, SIGNAL(toConsole(QString)), this, SLOT(consoleForward(QString)));
    }

    else if(mode == LISTENING){
        this->uds_listening = new UDS_Listening(0x0);

        // GUI Console Print
        connect(uds_listening, SIGNAL(toConsole(QString)), this, SLOT(consoleForward(QString)));
    }

    else if(mode == MCUISOTP){
        this->mcu_isotp = new ECU_ISOTP(0x1);

        // GUI Console Print
        connect(mcu_isotp, SIGNAL(toConsole(QString)), this, SLOT(consoleForward(QString)));
    }

    // Set the testcase
    this->testcase = mode;

}

void Testcasecontroller::startTests(){
    if(this->testcase == SELFTEST){
        selftest->startTests();
    }

    else if(this->testcase == LISTENING){
        uds_listening->startTests();
    }

    else if(this->testcase == MCUISOTP){
        mcu_isotp->startTests();
    }
}

//////////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////////

void Testcasecontroller::cleanUpTestcases(){
    if(this->selftest != nullptr){
        delete this->selftest;
        this->selftest = nullptr;
    }

    if(this->uds_listening != nullptr){
        delete this->uds_listening;
        this->uds_listening = nullptr;
    }

    if(this->mcu_isotp != nullptr){
        delete this->mcu_isotp;
        this->mcu_isotp = nullptr;
    }
}

//============================================================================
// Slots
//============================================================================

void Testcasecontroller::consoleForward(const QString &console){
    emit toConsole(console);
}
