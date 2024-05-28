// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : testcasecontroller.hpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Testcase Controller for different UDS tests
//============================================================================

#ifndef TESTCASECONTROLLER_H_
#define TESTCASECONTROLLER_H_

#include <QObject>
#include <QByteArray>

#include "Testcases/selftest.hpp"
#include "Testcases/uds_listening.hpp"
#include "Testcases/ecu_isotp.hpp"

class Testcasecontroller : public QObject{
    Q_OBJECT

public:
    enum TESTMODES {SELFTEST, LISTENING, MCUISOTP, GUITEST, MCUTEST};

private:
    Testcasecontroller::TESTMODES testcase;
    Selftest *selftest;
    UDS_Listening *uds_listening;
    ECU_ISOTP *mcu_isotp;

public:
    Testcasecontroller();
    virtual ~Testcasecontroller();

    void setTestMode(Testcasecontroller::TESTMODES mode);
    void startTests();

private:
    void cleanUpTestcases();

signals:
    /**
     * @brief Signals a Text to be print to GUI console
     */
    void toConsole(const QString &);


public slots:
    /**
     * @brief Slot for forwarding a Text to be print to GUI console
     */
    void consoleForward(const QString &console);

};

#endif /* TESTCASECONTROLLER_H_ */
