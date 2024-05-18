// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : testcases.cpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Testcases for UDS Communication
//============================================================================

#include "testcases.h"
#include <stdio.h>

#include "../WINDOWS_GUI/UDS_Spec/uds_comm_spec.h"

#include <QTime>

Testcases::Testcases() {
    this->ecu_id = 0x001;
    this->last_received = nullptr;
}

Testcases::~Testcases() {

}

void Testcases::setUDS(UDS* u) {
    uds = u;
}

void Testcases::run(){

    this->startCommunicationTests();
}

void Testcases::startCommunicationTests(){
    int waittime = 1000; // ms to wait
    int received_in_time = 0;
    QTime t = QTime();
    uint8_t* msg;
    int len;

    printf("Testcases: Starting basic communication test\n\n");

    printf("Testcase: Tester Present\n");
    msg = _create_tester_present(&len, 0, 1);
    UDS_Msg check = UDS_Msg(ecu_id, msg, len);

    last_received = nullptr;
    received_in_time = 0;
    int current = t.currentTime().msec();

    uds->testerPresent(ecu_id);
    while((t.currentTime().msec() - current) > waittime && received_in_time == 0){
        if(last_received != nullptr){
            received_in_time = 1;
            printf("Received Message in time\n");
        }
    }
    //this->checkEqual(last_received, &check);
}

void Testcases::messageInterpreter(UDS_Msg msg){

    printf(">> Testcases: Received Msg from 0x%08X: ", msg.getID());
    uint32_t len = 0;
    uint8_t* data = msg.getData(&len);
    for(auto i = 0; i < len; i++){
        printf("0x%02X ", data[i]);
    }
    printf("\n");

    // Set the last UDS Msg variable
    this->last_received = &msg;
}

uint8_t Testcases::checkEqual(UDS_Msg* rec, UDS_Msg* check){

    // Extract messages
    uint32_t len_rec = 0;
    uint8_t* data_rec = rec->getData(&len_rec);

    uint32_t len_check = 0;
    uint8_t* data_check = check->getData(&len_check);

    if(data_rec == nullptr){
        printf("Testcase - CHECK Messages: ERROR - Received Msg is nullptr");
        return 0;
    }

    if(data_check == nullptr){
        printf("Testcase - CHECK Messages: ERROR - Check Msg is nullptr");
        return 0;
    }

    if(len_rec != len_check){
        printf("Testcase - CHECK Messages: ERROR - Length is different");
        return 0;
    }

    for(auto i = 0; i < len_rec; i++){
        if(data_rec[i] != data_check[i]){
            printf("Testcase - CHECK Messages: ERROR - Content is different at index %d, Received: 0x%02X, Check: 0x%02X", i, data_rec[i], data_check[i]);
            return 0;
        }
    }
    printf("Testcase - CHECK Messages: Passed!");
    return 1;
}
