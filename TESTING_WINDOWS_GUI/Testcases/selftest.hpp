// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : selftest.hpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Class for Selftest testcases
//============================================================================

#ifndef SELFTEST_H_
#define SELFTEST_H_

#include "../testcase.hpp"

class Selftest : public Testcase {

public:
    Selftest(uint8_t gui_id);
    ~Selftest();

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

#endif /* SELFTEST_H_ */
