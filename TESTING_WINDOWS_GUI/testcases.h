// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : testcases.h
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Testcases for UDS Communication
//============================================================================


#ifndef TESTCASES_H
#define TESTCASES_H

#include <QThread>
#include <QString>

#include "stdint.h"
#include "../WINDOWS_GUI/UDS_Layer/UDS_Event.h"
#include "../WINDOWS_GUI/UDS_Layer/UDS.h"

class Testcases : public UDS_Event_Handler, public QObject{

private:
    uint32_t ecu_id;
    UDS* uds;
    UDS_Msg* last_received;

public:
    Testcases();
    virtual ~Testcases();

    void setUDS(UDS* u);

    void run();
    void messageInterpreter(UDS_Msg msg);

private:
    void startCommunicationTests();
    uint8_t checkEqual(UDS_Msg* rec, UDS_Msg* check);

};

#endif /* TESTCASES_H*/
