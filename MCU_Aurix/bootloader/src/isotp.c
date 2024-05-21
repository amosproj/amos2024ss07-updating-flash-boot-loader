//============================================================================
// Name        : isotp.c
// Author      : Leon Wilms
// Version     : 0.1
// Copyright   : MIT
// Description : ISO-TP stack
//============================================================================
#include "isotp.h"


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
    iso->flow_flag = 0;          // Flow control flags
    iso->bs = 0;                 // Block Size
    iso->stmin = 0;              // Separation Time Minimum
    iso->timer = 0;             // Timer for separation time

    return iso;
}

void isotp_send(isoTP* iso, uint8_t* data, uint32_t data_in_len){

    uint8_t* first_frame = tx_starting_frame(&iso->data_out_len,
                                                &iso->has_next,
                                                iso->max_len_per_frame,
                                                data,
                                                data_in_len,
                                                &iso->data_out_idx_ctr);

    if (first_frame == NULL) {
        // error handling

        return;
    }

    canTransmitMessage(0x123, first_frame, iso->data_out_len);
    free(first_frame);

    //implement flow control

    // Send consecutive frames if necessary
    while (iso->has_next) {

        // Wait for flow control if needed
        while (iso->flow_flag == 0x01) {

            waitTime(IfxStm_getTicksFromMilliseconds(BSP_DEFAULT_TIMER, 1)); // Wait 1 ms
        }


        uint8_t* consecutive_frame = tx_consecutive_frame(&iso->data_out_len,
                                                            &iso->has_next,
                                                            iso->max_len_per_frame,
                                                            data,
                                                            data_in_len,
                                                            &iso->data_out_idx_ctr,
                                                            &iso->frame_idx);

        if (consecutive_frame == NULL) {
            // error handling

            return;
        }

        canTransmitMessage(0x123, consecutive_frame, iso->data_out_len);
        free(consecutive_frame);

        // Handle block size (BS)
        if (iso->bs > 0 && iso->frame_idx % iso->bs == 0) {
            iso->flow_flag = 0x01; // Wait for flow control
        }

        //TODO: cleanup
        // Handle separation time (STmin)
        if (iso->stmin > 0) {
            waitTime(IfxStm_getTicksFromMilliseconds(BSP_DEFAULT_TIMER, iso->stmin)); // Wait STmin milliseconds
        }
    }
}

void isotp_recv(){


}

// TODO: not ready
void handle_flow_control_frame(isoTP* iso, uint8_t* flow_control_frame) {
    // Extract PCI type from the flow control frame
    uint8_t pci = flow_control_frame[0];

    // Update Block Size and Separation Time from the flow control frame
    iso->bs = flow_control_frame[1];
    iso->stmin = flow_control_frame[2];

    switch (pci) {
        case 0x30: // Continue to Send
            iso->flow_flag = 0x00;
            break;
        case 0x31: // Wait
            iso->flow_flag = 0x01;
            break;
        case 0x32: // Overflow/Abort
            iso->flow_flag = 0x02;
            break;
        default:
            // Invalid or unknown PCI type
            iso->flow_flag = 0x02;
            break;
    }
}

void isotp_free(isoTP* iso){

    free(iso);
}