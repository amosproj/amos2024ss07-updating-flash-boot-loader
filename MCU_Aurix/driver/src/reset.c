// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Wiktor Pilarczyk <wiktorpilar99@gmail.com>

//============================================================================
// Name        : reset.h
// Author      : Wiktor Pilarczyk
// Version     : 0.1
// Copyright   : MIT
// Description : reset C file for  for TC375 LK
//============================================================================

#include "reset.h"
#include "IfxScuRcu.h"
#include "SCU_Reset.h"

void softReset() {
    triggerSwReset(IfxScuRcu_ResetType_application);
}

void hardReset() {
    // TODO
}