// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Sebastian Rodriguez <r99@melao.de>

//============================================================================
// Name        : uds.c
// Author      : Sebastian Rodriguez
// Version     : 0.1
// Copyright   : MIT
// Description : UDS Layer implementation
//============================================================================

#include "uds.h"
#include "isotp.h"

void uds_diagnostic_session_control(uint8_t session){
    isoTP* iso = isotp_init();
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;
    int len;
    uint8_t *msg = _create_diagnostic_session_control(&len, 1, session);
    isotp_send(iso, msg, len);
    free(msg);
    isotp_free(iso);
}

void uds_ecu_reset(){
    isoTP* iso = isotp_init();
    iso->max_len_per_frame = MAX_FRAME_LEN_CAN;
    int len;
    uint8_t *msg = _create_ecu_reset(&len, 1, );
    isotp_send(iso, msg, len);
    free(msg);
    isotp_free(iso);
}




