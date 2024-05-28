// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : mcu_isotp.hpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Class for MCU ISO TP Test
//============================================================================

#ifndef ECU_ISOTP_H_
#define ECU_ISOTP_H_

#include "../testcase.hpp"

class ECU_ISOTP : public Testcase {

public:
    ECU_ISOTP(uint8_t gui_id);
    ~ECU_ISOTP();

    void messageChecker(const unsigned int id, const QByteArray &rec) override;
    void startTests() override;

private:
    // Specification for Diagnostic and Communication Management
    void testTesterPresent();

    // Specification for Data Transmission
    void testWriteDataByIdentifier();
};

#endif /* ECU_ISOTP_H_ */
