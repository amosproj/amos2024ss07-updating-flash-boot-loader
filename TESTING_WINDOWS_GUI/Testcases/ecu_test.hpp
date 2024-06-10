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

#define FBL_DID_SYSTEM_NAME_BYTES_SIZE                              (32)
#define FBL_DID_PROGRAMMING_DATE_BYTES_SIZE                         (6)
#define FBL_DID_BL_KEY_ADDRESS_BYTES_SIZE                           (4)
#define FBL_DID_BL_KEY_GOOD_VALUE_BYTES_SIZE                        (4)
#define FBL_DID_CAN_BASE_MASK_BYTES_SIZE                            (2)
#define FBL_DID_CAN_ID_BYTES_SIZE                                   (2)
#define FBL_DID_BL_WRITE_START_ADD_CORE0_BYTES_SIZE                 (4)
#define FBL_DID_BL_WRITE_END_ADD_CORE0_BYTES_SIZE                   (4)
#define FBL_DID_BL_WRITE_START_ADD_CORE1_BYTES_SIZE                 (4)
#define FBL_DID_BL_WRITE_END_ADD_CORE1_BYTES_SIZE                   (4)
#define FBL_DID_BL_WRITE_START_ADD_CORE2_BYTES_SIZE                 (4)
#define FBL_DID_BL_WRITE_END_ADD_CORE2_BYTES_SIZE                   (4)

#include "../testcase.hpp"

class ECU_Test : public Testcase {

private:
    bool writing_test;

    uint8_t did_system_name_len = 17; // Name including end of string sign
    uint8_t did_system_name[FBL_DID_SYSTEM_NAME_BYTES_SIZE] = "Another ECU Name";
    uint8_t did_programming_date[FBL_DID_PROGRAMMING_DATE_BYTES_SIZE] = {0x01, 0x06, 0x25, 0x12, 0x23, 0x45};
    uint8_t did_bl_key_address[FBL_DID_BL_KEY_ADDRESS_BYTES_SIZE] = {0x0A, 0xF4, 0x08, 0x90};
    uint8_t did_bl_key_good_value[FBL_DID_BL_KEY_GOOD_VALUE_BYTES_SIZE] = {0x39, 0x68, 0x3C, 0x5A};
    uint8_t did_can_base_mask[FBL_DID_CAN_BASE_MASK_BYTES_SIZE] = {0xFF, 0xFF};
    uint8_t did_can_id[FBL_DID_CAN_ID_BYTES_SIZE] = {0x00, 0x01};
    uint8_t did_bl_write_start_add_core0[FBL_DID_BL_WRITE_START_ADD_CORE0_BYTES_SIZE] = {0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t did_bl_write_end_add_core0[FBL_DID_BL_WRITE_END_ADD_CORE0_BYTES_SIZE] = {0x00, 0x00, 0x00, 0x00};
    uint8_t did_bl_write_start_add_core1[FBL_DID_BL_WRITE_START_ADD_CORE1_BYTES_SIZE] = {0xAA, 0xAA, 0xAA, 0xAA};
    uint8_t did_bl_write_end_add_core1[FBL_DID_BL_WRITE_END_ADD_CORE1_BYTES_SIZE]= {0xBB, 0xBB, 0xBB, 0xBB};
    uint8_t did_bl_write_start_add_core2[FBL_DID_BL_WRITE_START_ADD_CORE2_BYTES_SIZE]= {0xCC, 0xCC, 0xCC, 0xCC};
    uint8_t did_bl_write_end_add_core2[FBL_DID_BL_WRITE_END_ADD_CORE2_BYTES_SIZE]= {0xDD, 0xDD, 0xDD, 0xDD};


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
