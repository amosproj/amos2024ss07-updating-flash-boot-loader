// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : ecu_test.hpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Class for ECU testcases
//============================================================================

#ifndef ECU_TEST_H_
#define ECU_TEST_H_

#include "../testcase.hpp"

class ECU_Test : public Testcase {

public:
    ECU_Test(uint8_t gui_id);
    ~ECU_Test();

    void messageChecker(const unsigned int id, const QByteArray &rec) override;
    void startTests() override;

private:
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
};

#endif /* ECU_TEST_H_ */
