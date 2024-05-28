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

    uint16_t data_in_len;
    uint8_t ready_to_read; // bool that will be set to =! 0 if message can be read.

}isoTP_RX;


isoTP* isotp_init(void);

void isotp_send(isoTP* iso, uint8_t* data, uint32_t data_in_len);

uint8_t* isotp_rcv(int16_t* total_length);



void rx_reset_isotp_buffer();

void tx_reset_isotp_buffer(isoTP* iso);

void process_can_to_isotp(uint32_t* rxData, IfxCan_DataLengthCode dlc);

void close_isoTP(isoTP* iso);

#endif /* BOOTLOADER_INC_ISOTP_H_ */
