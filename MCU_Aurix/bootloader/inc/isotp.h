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
#include "can_init.h"

// TODO: I changed 'data_out_len' and 'has_next' to uint32_t. Will this still work?
typedef struct isoTp
{

    // uds_comm_spec fields
    uint32_t data_out_len;
    uint32_t has_next;
    uint8_t frame_idx;
    uint32_t data_out_idx_ctr;

    // Flow control fields
    uint8_t flow_flag;          // Flow control flags
    uint8_t bs;                 // Block Size
    uint8_t stmin;              // Separation Time Minimum
    uint32_t timer;             // Timer for separation time

    //this needs to be changed for transmission
    uint8_t max_len_per_frame;

}isoTP;

typedef struct isoTp_RX
{

    uint8_t* data;
    uint8_t* write_ptr;


}isoTP_RX;


isoTP* isotp_init();

void isotp_send(isoTP* iso, uint8_t* data, uint32_t data_in_len);

void isotp_recv();

void isotp_free(isoTP* iso);

#endif /* BOOTLOADER_INC_ISOTP_H_ */
