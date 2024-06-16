// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Leon Wilms <leonwilms.wk@gmail.com>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : isotp.c
// Author      : Leon Wilms, Michael Bauer
// Version     : 0.2
// Copyright   : MIT
// Description : ISO-TP stack
//============================================================================
//TODO: Move isoTPRX buffer as global variable~LEON - Would keep it in class file, no need to have it globally ~Michael
#include "isotp.h"
#include "uds.h"
#include "memory.h"

isoTP_RX* iso_RX;
isoTP* iso_TX;
uint8_t isoTP_RX_data_buffer[MAX_ISOTP_MESSAGE_LEN];

//============================================================================
// Init / Deinit / Resetting
//============================================================================

/*
 * @brief                       This function resets the internal receive buffer for isoTP.
 *
 */
void rx_reset_isotp_buffer(){

    //memset(iso_RX->data, 0, MAX_ISOTP_MESSAGE_LEN);

    iso_RX->write_ptr = iso_RX->data;
    iso_RX->data_in_len = 0;
    iso_RX->ready_to_read = 0;
    iso_RX->last_consecutive_ctr = 0;
}

/*
 * @brief                       This function resets the given transmit buffer for isoTP.
 *
 */
void tx_reset_isotp_buffer(isoTP* iso){

    // Do not change these;
    iso->data_out_len = 0;
    iso->has_next = 0;
    iso->frame_idx = 0;
    iso->data_out_idx_ctr = 0;

    iso->flow_flag = 0;          // Flow control flags
    iso->bs = 0;                 // Block Size
    iso->stmin = 0;              // Separation Time Minimum
    iso->timer = 0;              // Timer for separation time

    iso->max_len_per_frame = 0;  //Depending to the protcol change this
}

/*
 * @brief                       This function initializes the isoTP layer.
 *
 * @return                      Returns a isoTP struct used in the sending logic.
 */
isoTP* isotp_init(){

    canInitDriver(process_can_to_isotp);

    // Init the isoTP struct for TX, will be used as return
    isoTP* isotp_TX = malloc(sizeof(isoTP));
    if (isotp_TX == NULL){
        return NULL;
    }
    tx_reset_isotp_buffer(isotp_TX);

    // Init the isoTP struct for RX
    isoTP_RX* isotp_RX = malloc(sizeof(isoTP_RX));
    if (isotp_RX == NULL){
        free(isotp_TX);
        return NULL;
    }

    // Assign the RX data buffer
    isotp_RX->data = isoTP_RX_data_buffer;
    isotp_RX->write_ptr = isoTP_RX_data_buffer;
    iso_RX = isotp_RX;

    return isotp_TX;
}

/*
 * @brief                       This function free's all the allocated data on the isoTP layer,
 *                              effectively closing the isoTP layer.
 *
 */
void close_isoTP(isoTP* iso){

    free(iso);

    free(iso_RX->data);
    free(iso_RX);
}

//============================================================================
// TX
//============================================================================

/*
 * @brief                       This function sends data from the isoTP layer.
 *
 * @param iso                   This is a pointer the isoTP struct, which is used for sending.
 *                              Please edit the value iso->max_len_per_frame depending on used transmission protocol.
 *
 * @param data                  Pointer to the data.
 *
 * @param data_in_len           Length of the data to be sent.
 *
 */
void isotp_send(isoTP* iso, uint8_t* data, uint32_t data_in_len){

    iso_TX = iso;

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

    // Directly read ECU ID from Memory function
    canTransmitMessage(getID(), first_frame, iso->data_out_len);
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

        // Directly read ECU ID from Memory function
        canTransmitMessage(getID(), consecutive_frame, iso->data_out_len);
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

    iso_TX = NULL;
}

//============================================================================
// RX
//============================================================================

/*
 * @brief                       This function returns the received isoTP message if it is ready.
 *
 * @param total_length          This is a pointer to the total length of the isoTP message.
 *
 * @return                      Returns the isoTP message, else NULL.
 *
 *                              Will also set total_length in special cases:
 *                              -1: ERROR.
 *                              0:  Buffer is not ready to be read.
 *
 *
 */
//TODO: Add flow control logic
uint8_t* isotp_rcv(uint32_t* total_length){
    //Caller has to free the returned message

    // Error: iso_RX is not properly initialized
    if (iso_RX == NULL || iso_RX->data == NULL || iso_RX->write_ptr == NULL) {

        *total_length = 0;
        return NULL;
    }

    // isoTP message still not ready to be read
    if (iso_RX->ready_to_read == 0){

        *total_length = 0;
        return NULL;
    }

    // Get message size of whole isoTP message
    *total_length = iso_RX->write_ptr - iso_RX->data;

    // Create and write buffer for the specific isoTP message received.
    uint8_t* uds_message = calloc(*total_length, sizeof(uint8_t));
    memcpy(uds_message, iso_RX->data, *total_length);

    rx_reset_isotp_buffer();

    return uds_message;
}

//============================================================================
// Processing
//============================================================================


/*
 * @brief                       This function extracts the isoTP message from multiple CAN messages.
 *                              It is called inside the receive interrupt 'canIsrRxFifo0Handler' of the CAN driver.
 *
 * @param rxData                This is a pointer to the received data.
 *
 * @param dlc                   Data Length Code, this represents the length of the currently received CAN message.
 *
 */
void process_can_to_isotp(uint32_t* rxData, IfxCan_DataLengthCode dlc){

    if(dlc <= 0) // Need to assume that the data is also empty, using FBL_NEGATIVE_RESPONSE instead
        uds_neg_response(FBL_NEGATIVE_RESPONSE, FBL_RC_INCORRECT_MSG_LEN_OR_INV_FORMAT);

    uint8_t* data_ptr = (uint8_t*)rxData;

    if(MAX_ISOTP_MESSAGE_LEN - (iso_RX->write_ptr - iso_RX->data) < dlc || iso_RX->ready_to_read != 0){

        // Request with negative response in case of error
        if(rx_is_single_Frame(data_ptr, dlc, MAX_FRAME_LEN_CAN))
            uds_neg_response(data_ptr[1], FBL_RC_BUSY_REPEAT_REQUEST);
        else // Using FBL_NEGATIVE_RESPONSE since it is any other frame (Consecutive or Flow Control)
            uds_neg_response(FBL_NEGATIVE_RESPONSE, FBL_RC_BUSY_REPEAT_REQUEST);

        return;
    }


    //TODO: Check for flow control and handle  (in case we send data)
    // Single Frame get length of whole isoTP message
    if(((0xF0 & rxData[0]) >> 4) == 0){

        iso_RX->data_in_len = 0xF & data_ptr[0];

        memcpy(iso_RX->write_ptr, &data_ptr[1], dlc - 1);
        iso_RX->write_ptr += dlc - 1;
    }
    // First Frame get length of whole isoTP message
    else if(((0xF0 & rxData[0]) >> 4) == 1){

        iso_RX->data_in_len = ((data_ptr[0] & 0x0F) << 4) | data_ptr[1];

        memcpy(iso_RX->write_ptr, &data_ptr[2], dlc - 2);
        iso_RX->write_ptr += dlc - 2;

        // Send Flow Control Frame as response
        uint32_t flow_ctrl_len = 0;
        uint8_t *flow_ctrl = tx_flow_control_frame(&flow_ctrl_len, 0, 0, 0, 0);
        canTransmitMessage(getID(), flow_ctrl, flow_ctrl_len);
        free(flow_ctrl);

    }
    // Consecutive Frame
    else if(((0xF0 & rxData[0]) >> 4) == 2){

        // Only copy if counter is different. Sender needs to make sure that correct sequence is transmitted
        if(iso_RX->last_consecutive_ctr != &data_ptr[0]){
            memcpy(iso_RX->write_ptr, &data_ptr[1], dlc - 1);
            iso_RX->write_ptr += dlc - 1;
        }

        // Send response for Consecutive Frame
        canTransmitMessage(getID(), &data_ptr[0], 1);

        // Store the counter
        iso_RX->last_consecutive_ctr = data_ptr[0];
    }
    // Flow Control
    else if(((0xF0 & rxData[0]) >> 4) == 3){

        // Check for current isoTP TX and set bytes accordingly
        if(iso_TX != NULL){
            // TODO: Implement UDS Comm Spec method to readout the variables and fill iso_TX
        }
    }
    // ERROR
    else{
        uds_neg_response(data_ptr[0], FBL_RC_BUSY_REPEAT_REQUEST);
    }

    // Set ready_to_read if all bytes have been received for one isoTP message
    if(iso_RX->write_ptr - iso_RX->data >= iso_RX->data_in_len){

        iso_RX->ready_to_read = 1;
    }

    return;
}
