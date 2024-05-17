// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : uds_event_handler.h
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Interface Class for UDS Events
//============================================================================


#ifndef UDS_EVENT_H
#define UDS_EVENT_H

#include "UDSMsg.h"

class UDS_Event_Handler
{
public:
    UDS_Event_Handler();
    virtual ~UDS_Event_Handler();

    virtual void messageInterpreter(UDS_Msg msg);
};

#endif // UDS_EVENT_H
