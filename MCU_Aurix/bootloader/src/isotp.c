//============================================================================
// Name        : isotp.c
// Author      : Leon Wilms
// Version     : 0.1
// Copyright   : MIT
// Description : ISO-TP stack
//============================================================================
#include "isotp.h"

//TODO: change int* to an uintX_t standard



isoTP* isotp_init(){

    isoTP* iso = calloc(1, sizeof(isoTP));
    if (iso == NULL){

        return NULL;
    }

    // Do not change these;
    iso->data_out_len = 0;
    iso->has_next = 0;
    iso->frame_idx = 0;
    iso->data_out_idx_ctr = 0;


    //Depending to the protcol change this
    iso->max_len_per_frame = 0;

    return iso;
}

void isotp_send(isoTP* iso, uint8_t* data, uint32_t data_in_len){


    uint8_t* first_frame = tx_starting_frame(&iso->data_out_len,
                                                &iso->has_next,
                                                iso->max_len_per_frame,
                                                data,
                                                data_in_len,
                                                &iso->data_out_idx_ctr);

    canTransmitMessage(0x0, first_frame, iso->data_out_len);
    free(first_frame);

    //implement flow control

    // Send consecutive frames if necessary
    while (iso->has_next) {
        uint8_t* consecutive_frame = tx_consecutive_frame(&iso->data_out_len,
                                                            &iso->has_next,
                                                            iso->max_len_per_frame,
                                                            data,
                                                            data_in_len,
                                                            &iso->data_out_idx_ctr,
                                                            &iso->frame_idx);

        canTransmitMessage(0x0, consecutive_frame, iso->data_out_len);
        free(consecutive_frame);
    }
}

void isotp_recv(){


}

void isotp_free(isoTP* iso){

    free(iso);
}
