//============================================================================
// Name        : isotp.h
// Author      : Leon Wilms
// Version     : 0.1
// Copyright   : MIT
// Description : ISO-TP stack
//============================================================================
#ifndef BOOTLOADER_INC_ISOTP_H_
#define BOOTLOADER_INC_ISOTP_H_


#include "can_driver.h"
#include "uds_comm_spec.h"

typedef enum {
    CAN = 8,

    //TODO: change max frame size for FD and Ethernet
    CAN_FD = 10,
    Ethernet = 15
} CommunicationType;

//TODO: change int* to an uintX_t standard

typedef struct isoTp
{
    int data_out_len;
    int has_next;
    uint8_t frame_idx;
    uint32_t data_out_idx_ctr;

    uint8_t max_len_per_frame;

}isoTP;


isoTP* isotp_init();

void isotp_send(isoTP* iso, uint8_t* data, uint32_t data_in_len);

void isotp_recv();

void isotp_free(isoTP* iso);

#endif /* BOOTLOADER_INC_ISOTP_H_ */
