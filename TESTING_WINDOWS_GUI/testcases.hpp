// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : testcases.hpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Testcases for UDS selftests, GUI tests and MCU tests
//============================================================================
#ifndef TESTCASES_H_
#define TESTCASES_H_

#include <QObject>
#include <QByteArray>

#include "stdint.h"

#include "../WINDOWS_GUI/UDS_Layer/UDS.hpp"
#include "../WINDOWS_GUI/Communication_Layer/Communication.hpp"

class Testcases : public QObject{
    Q_OBJECT

public:
    enum TESTMODES {SELFTEST, GUITEST, MCUTEST, LISTENING, MCUISOTP};

private:
    uint32_t ecu_id;
    uint8_t gui_id, no_gui_id;
    Testcases::TESTMODES testmode;

    Communication *comm;
    UDS *uds;


public:
    Testcases();
	virtual ~Testcases();

    void setTestMode(Testcases::TESTMODES mode);
    void startTests();
    void messageChecker(const unsigned int id, const QByteArray &rec);

    // Sending out broadcast for tester present
	void testReqIdentification(); 

	// Specification for Diagnostic and Communication Management
	void testDiagnosticSessionControl();
	void testEcuReset();
	void testSecurityAccessRequestSEED();
	void testSecurityAccessVerifyKey();
	void testTesterPresent();

	// Specification for Data Transmission
	void testReadDataByIdentifier();
	void testReadMemoryByAddress();
	void testWriteDataByIdentifier();

	// Specification for Upload | Download
	void testRequestDownload();
	void testRequestUpload();
	void testTransferData();
	void testRequestTransferExit();

	// Supported Common Response Codes
	void testNegativeResponse();


private:
	uint32_t createCommonID(uint32_t base_id, uint8_t gui_id, uint32_t ecu_id);
    uint8_t checkEqual(unsigned int recid, const QByteArray &rec, unsigned int checkid, QByteArray &check);

signals:
    /**
     * @brief Signals a Text to be print to GUI console
     */
    void toConsole(const QString &);



public slots:
    /**
     * @brief Slot for received UDS Message to be checked
     * @param id ID of Sender
     * @param ba ByteArray with received data
     */
    void rxDataReceiverSlot(const unsigned int id, const QByteArray &ba);

    /**
     * @brief Slot for forwarding a Text to be print to GUI console
     */
    void consoleForward(const QString &console);

};

#endif /* TESTCASES_H_ */
