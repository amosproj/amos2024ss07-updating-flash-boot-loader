// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : uds_listening.hpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Class for UDS Listening only
//============================================================================

#ifndef UDS_LISTENING_H_
#define UDS_LISTENING_H_

#include "../testcase.hpp"

class UDS_Listening : public Testcase {

public:
    UDS_Listening(uint8_t gui_id);
    ~UDS_Listening();

    void messageChecker(const unsigned int id, const QByteArray &rec) override;
    void startTests() override;

};

#endif /* UDS_LISTENING_H_ */
