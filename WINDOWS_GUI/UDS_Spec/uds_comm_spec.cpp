// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>

//============================================================================
// Name        : uds_comm_spec.c
// Author      : Michael Bauer, Leon Wilms, Wiktor Pilarczyk
// Version     : 1.0
// Copyright   : MIT
// Description : UDS communication specification implementation
//============================================================================


#ifdef __cplusplus
extern "C" {
#endif

#include "uds_comm_spec.h"
#include <stdlib.h>
#include <stdio.h>

// TODO: Check on Error Handling for calloc -> Mainly relevant for MCU

//////////////////////////////////////////////////////////////////////////////
// ISO TP Handling - TX
//////////////////////////////////////////////////////////////////////////////

uint8_t *tx_starting_frame(uint32_t *data_out_len, uint32_t *has_next, uint8_t max_len_per_frame, uint8_t* data_in, uint32_t data_in_len, uint32_t* data_out_idx_ctr){
    // Caller need to free the memory after processing
    uint8_t can = (max_len_per_frame <= MAX_FRAME_LEN_CAN);

    if (data_in_len == 0){
        *data_out_len = 0;
        uint8_t *msg = (uint8_t*)calloc(*data_out_len, sizeof(uint8_t));
        *data_out_idx_ctr = 0; // No further frame
        *has_next = 0;
        return msg;
    }

    if (can){
        if (data_in_len < max_len_per_frame){
            // Generate Single Frame (Code = 0; Size 0-7) -> Data consists of 0 - 7 bytes of payload
            *data_out_len = data_in_len + 1; // Include Protocol Control Information (PCI)
            *has_next = 0;       // No further frames necessary

            uint8_t *msg = (uint8_t*)calloc(*data_out_len, sizeof(uint8_t));

            if (msg == NULL){
                *data_out_len = 0;
                return msg;
            }

            msg[0] = (uint8_t)(data_in_len & 0xF); // PCI
            for(uint32_t i = 0; i < data_in_len; i++)
                msg[1+i] = data_in[i];  // Payload
            *data_out_idx_ctr = 0; // No further frame
            return msg;
        }
        else {
            // Generate First Frame (Code = 1, Data 0+1 (=0x1008..0x1FFF)) -> Data consists of initial payload (6 bytes), Size 8-4095
            *data_out_len = max_len_per_frame; // Include Protocol Control Information (PCI)
            *has_next = 1;      // Further frames necessary

            uint8_t *msg = (uint8_t*)calloc(*data_out_len, sizeof(uint8_t));

            if (msg == NULL){
                *data_out_len = 0;
                return msg;
            }

            msg[0] = (1<<4);    // PCI

            if (data_in_len > 0xFFF)    // Restrict length to max length
                data_in_len = 0xFFF;

            msg[0] |= (uint8_t)(data_in_len>>8 & 0xF);
            msg[1] = (uint8_t)(data_in_len & 0xFF);

            *data_out_idx_ctr = 6; // Add the first 6 bytes, next frame starts at idx 6
            for(uint32_t i = 0; i < *data_out_idx_ctr; i++)
                msg[2+i] = data_in[i];  // Payload
            return msg;
        }
    }

    // Other, e.g. CAN FD
    else {
        printf("TODO: Not yet implemented!");
        *data_out_len = 0;
        *has_next = 0;
        return (uint8_t*)calloc(0, sizeof(uint8_t));
    }
}

uint8_t *tx_consecutive_frame(uint32_t *data_out_len, uint32_t *has_next, uint8_t max_len_per_frame, uint8_t* data_in, uint32_t data_in_len, uint32_t* data_out_idx_ctr, uint8_t* frame_idx){
    // Caller need to free the memory after processing
    uint8_t can = (max_len_per_frame <= MAX_FRAME_LEN_CAN);

    if (data_in_len == 0){
        *data_out_len = 0;
        uint8_t *msg = (uint8_t*)calloc(*data_out_len, sizeof(uint8_t));
        *has_next = 0;
        return msg;
    }

    if (can){
        if (data_in_len > 0xFFF)    // Restrict length to max length
            data_in_len = 0xFFF;

        // Generate Consecutive Frame (Code = 2, Index 1-15)
        if((*data_out_idx_ctr + (max_len_per_frame-1)) < data_in_len){
            *data_out_len = max_len_per_frame; // Include Protocol Control Information (PCI)
            *has_next = 1;
        }
        else { // Last Frame reached
            *data_out_len = data_in_len - *data_out_idx_ctr + 1;
            *has_next = 0;
        }

        uint8_t *msg = (uint8_t*)calloc(*data_out_len, sizeof(uint8_t));
        msg[0] = (2<<4);    // PCI
        *frame_idx += 1;
        if ((*frame_idx % 0x10) == 0)
            *frame_idx = 1;
        msg[0] |= *frame_idx;

        for(uint8_t i = 0; i < *data_out_len-1; i++){
            msg[1+i] = data_in[*data_out_idx_ctr];
            *data_out_idx_ctr += 1;
        }
        return msg;
    }

    printf("TODO: Not yet implemented!");
    *data_out_len = 0;
    *has_next = 0;
    return (uint8_t*)calloc(0, sizeof(uint8_t));
}

// TODO: think about a better solution with sep_time ~ Leon
/*
 * @brief                       This function creates the flow control message which has to be sent.
 *                              Caller needs to free the memory after processing.
 *
 * @param data_out_len          Points to the final length of this message,
 *                              will be set in the function.
 * @param flag                  0x00 = Continue To Send, 0x01 = Wait, 0x02 = Overflow / Abort
 *
 * @param blocksize
 *
 * @param sep_time_millis       For values 0-127, will wait for 0-127 milli-seconds.
 *
 * @param sep_time_multi_micros For values 0x01-0x09, will wait for 1-9 x 100 micro-seconds
 *                              and overshadow 'sep_time_millis'.
 *                              If set to 0, this will be ignored. No values above 9.
 *
 * @return                      Returns dynamically allocated flow control frame.
 *
 */
uint8_t *tx_flow_control_frame(uint32_t *data_out_len, uint8_t flag, uint8_t blocksize, uint8_t sep_time_millis, uint8_t sep_time_multi_micros){
    // Caller need to free the memory after processing
    *data_out_len = 3;
    uint8_t *msg = (uint8_t*)calloc(*data_out_len, sizeof(uint8_t));

    if (msg == NULL){
        *data_out_len = 0;
        return msg;
    }

    msg[0] = (3<<4);
    msg[0] |= (flag & 0x3);
    msg[1] = blocksize;

    if(sep_time_multi_micros){
        if(sep_time_multi_micros > 0x9)
            sep_time_multi_micros = 0x9;
        msg[2] = 0xF0 | sep_time_multi_micros;
    }
    else {
        msg[2] = 0x7F & sep_time_millis;
    }

    return msg;
}

//////////////////////////////////////////////////////////////////////////////
// ISO TP Handling - RX
//////////////////////////////////////////////////////////////////////////////

uint8_t rx_is_starting_frame(uint8_t* data_in, uint32_t data_in_len, uint8_t max_len_per_frame){
    uint8_t can = (max_len_per_frame <= MAX_FRAME_LEN_CAN);

    if (data_in_len == 0)
        return 0xFF; // Error

    if(can){
        uint8_t result = (((0xF0 & data_in[0])>> 4) == 0) || (((0xF0 & data_in[0])>> 4) == 1);
        //printf("UDS_Comm_Spec - rx_is_starting_frame: %d\n", result);
        return result;
    }

    printf("TODO: Not yet implemented!");
    return 0xFF; // Error
}

uint8_t rx_is_consecutive_frame(uint8_t* data_in, uint32_t data_in_len, uint8_t max_len_per_frame){
    uint8_t can = (max_len_per_frame <= MAX_FRAME_LEN_CAN);

    if (data_in_len == 0)
        return 0xFF; // Error

    if(can){
        uint8_t result = ((0xF0 & data_in[0])>> 4) == 2;
        //printf("UDS_Comm_Spec - rx_is_consecutive_frame: %d\n", result);
        return result;
    }

    printf("TODO: Not yet implemented!");
    return 0xFF; // Error
}

uint8_t rx_is_single_Frame(uint8_t* data_in, uint32_t data_in_len, uint8_t max_len_per_frame){
    uint8_t can = (max_len_per_frame <= MAX_FRAME_LEN_CAN);

    if (data_in_len == 0)
        return 0xFF; // Error

    if(can){
        uint8_t result = (((0xF0 & data_in[0])>> 4) == 0);
        //printf("UDS_Comm_Spec - rx_is_single_frame: %d\n", result);
        return result;
    }

    printf("TODO: Not yet implemented!");
    return 0xFF; // Error
}

uint8_t rx_is_flow_control_frame(uint8_t* data_in, uint32_t data_in_len, uint8_t max_len_per_frame){
    uint8_t can = (max_len_per_frame <= MAX_FRAME_LEN_CAN);

    if (data_in_len == 0)
        return 0xFF; // Error

    if(can){
        uint8_t result = (((0xF0 & data_in[0])>> 4) == 3);
        //printf("UDS_Comm_Spec - rx_is_flow_control_frame: %d\n", result);
        return result;
    }

    printf("TODO: Not yet implemented!");
    return 0xFF; // Error
}

uint8_t *rx_starting_frame(uint32_t *data_out_len, uint32_t *has_next, uint8_t max_len_per_frame, uint8_t* data_in, uint32_t data_in_len){
    // Caller need to free the memory after processing
    uint8_t can = (max_len_per_frame <= MAX_FRAME_LEN_CAN);

    if (data_in_len == 0){
        *data_out_len = 0;
        uint8_t *msg = (uint8_t*)calloc(*data_out_len, sizeof(uint8_t));
        return msg;
    }

    if (can){
        if(((0xF0 & data_in[0]) >> 4) == 0){ // Single Frame
            *data_out_len = 0xF & data_in[0];
            uint8_t *msg = (uint8_t*)calloc(*data_out_len, sizeof(uint8_t));
            *has_next = 0;

            if (msg == NULL){
                *data_out_len = 0;
                return msg;
            }

            // Copy content
            for(unsigned int i = 0; i < *data_out_len; i++){
                msg[i] = data_in[i+1];
            }
            return msg;
        }

        else if(((0xF0 & data_in[0])>>4) == 1){ // First Frame
            *data_out_len = ((data_in[0] & 0x0F) << 4) | data_in[1];
            uint8_t *msg = (uint8_t*)calloc(*data_out_len, sizeof(uint8_t));
            *has_next = 1;

            if (msg == NULL){
                *data_out_len = 0;
                return msg;
            }

            // Copy content
            for(unsigned int i = 0; i < data_in_len - 2; i++){
                msg[i] = data_in[i+2];
            }
            return msg;
        }
        else{
            *data_out_len = 0;
            *has_next = 0;
            return (uint8_t*)calloc(0, sizeof(uint8_t));
        }
    }
    // Other, e.g. CAN FD
    else {
        printf("TODO: Not yet implemented!");
        *data_out_len = 0;
        *has_next = 0;
        return (uint8_t*)calloc(0, sizeof(uint8_t));
    }
}

uint8_t rx_consecutive_frame(uint32_t *data_out_len, uint8_t *data_out, uint32_t *has_next, uint32_t data_in_len, uint8_t* data_in, uint32_t *idx){

    if (data_in_len == 0){
        *has_next = 0;
        return 0;
    }

    if ((*idx + (data_in_len - 1)) > *data_out_len){
        *has_next = 0;
        printf("UDS Comm Spec Usage Error: Received data is too long for data buffer - IDX: %d, Data_In_Len: %d, Data_out_len: %d\n", *idx, data_in_len, *data_out_len);
        return 0;
    }

    // Write all the data that fits into available data_out buffer
    for(unsigned int i = 1; i < data_in_len; i++){
        data_out[*idx] = data_in[i];
        *idx = *idx + 1;
    }

    if (*idx < *data_out_len )
        *has_next = 1;
    else
        *has_next = 0;

    return 1;
}

//////////////////////////////////////////////////////////////////////////////
// Supported Service Overview (SID)
//////////////////////////////////////////////////////////////////////////////

uint8_t *prepare_message(int *len, uint8_t response, uint8_t SID, uint8_t info, uint32_t len_val) {
    // Caller need to free the memory after processing
    *len = len_val;
    uint8_t *msg = (uint8_t*)calloc(*len, sizeof(uint8_t));

    if (msg == NULL){
        *len = 0;
        return msg;
    }

    msg[0] = SID;                                               // SID
    if (response)
        msg[0] += FBL_SID_ACK;                                  // Response ACK
    msg[1] = info;
    return msg;
}

/**
 * Specification for Diagnostic and Communication Management
 */

//Diagnostic Session Control (0x10)
uint8_t *_create_diagnostic_session_control(int *len, uint8_t response, uint8_t session) {
    return prepare_message(len, response, FBL_DIAGNOSTIC_SESSION_CONTROL, session, 2);
}

//ECU Reset (0x11)
uint8_t *_create_ecu_reset(int *len, uint8_t response, uint8_t reset_type){
    return prepare_message(len, response, FBL_ECU_RESET, reset_type, 2);
}

// Security Access (0x27)
uint8_t *_create_security_access(int *len, uint8_t response, uint8_t request_type, uint8_t* key, uint8_t key_len) {
    uint8_t *msg = prepare_message(len, response, FBL_SECURITY_ACCESS, request_type, 2 + key_len);
    if (msg == NULL)
        return msg;

    for(int i = 0; i < key_len; i++)
        msg[2+i] = key[i];                                      // Payload
    return msg;
}

//Tester Present (0x3E)
uint8_t *_create_tester_present(int *len, uint8_t response, uint8_t response_type) {
    return prepare_message(len, response, FBL_TESTER_PRESENT, response_type, 2);
}

/**
 * Specification for Data Transmission
 */

// Read Data By Identifier (SID 0x22)
uint8_t *_create_read_data_by_ident(int *len, uint8_t response, uint16_t did, uint8_t* data, uint8_t data_len){
    uint8_t *msg = prepare_message(len, response, FBL_READ_DATA_BY_IDENTIFIER, 0, 3 + data_len);
    if (msg == NULL)
        return msg;

    msg[1] = (uint8_t)((did>>8) & 0xFF);                        // DID High Byte
    msg[2] = (uint8_t)((did)    & 0xFF);                        // DID Low Byte
    for(int i = 0; i < data_len; i++)
        msg[3+i] = data[i];                                     // Payload
    return msg;
}

// Read Data By Address (SID 0x23)
uint8_t *_create_read_memory_by_address(int *len, uint8_t response, uint32_t addr, uint16_t no_bytes, uint8_t* data, uint16_t data_len) {
    // Caller need to free the memory after processing
    *len = 7;
    if(data_len)
        *len += -2 + data_len;                                  // Without Number of Bytes
    uint8_t *msg = prepare_message(len, response, FBL_READ_MEMORY_BY_ADDRESS, 0, *len);
    if (msg == NULL)
        return msg;

    msg[1] = (uint8_t)((addr>>24) & 0xFF);                      // Address Byte 4
    msg[2] = (uint8_t)((addr>>16) & 0xFF);                      // Address Byte 3
    msg[3] = (uint8_t)((addr>>8)  & 0xFF);                      // Address Byte 2
    msg[4] = (uint8_t)((addr)     & 0xFF);                      // Address Byte 1
    if(!data_len){
        msg[5] = (uint8_t)((no_bytes>>8) & 0xFF);               // Number of Bytes Byte 1
        msg[6] = (uint8_t)((no_bytes)    & 0xFF);               // Number of Bytes Byte 0
    }
    else {
        for(int i = 0; i < data_len; i++)
            msg[5+i] = data[i];                                 // Payload
    }

    return msg;
}

// Write Data By Identifier (SID 0x2E)
uint8_t *_create_write_data_by_ident(int *len, uint8_t response, uint16_t did, uint8_t* data, uint8_t data_len) {
    uint8_t *msg = prepare_message(len, response, FBL_WRITE_DATA_BY_IDENTIFIER, 0, 3 + data_len);
    if (msg == NULL)
        return msg;

    msg[1] = (uint8_t)((did>>8) & 0xFF);                        // DID High Byte
    msg[2] = (uint8_t)((did)    & 0xFF);                        // DID Low Byte
    for(int i = 0; i < data_len; i++)
        msg[3+i] = data[i];                                     // Payload
    return msg;
}


/**
 * Specification for Upload | Download
 */

void upload_download_message(uint8_t *msg, int len, uint32_t addr, uint8_t response, uint32_t bytes_size) {
    msg[1] = (uint8_t)((addr>>24) & 0xFF);                      // Address Byte 4
    msg[2] = (uint8_t)((addr>>16) & 0xFF);                      // Address Byte 3
    msg[3] = (uint8_t)((addr>>8)  & 0xFF);                      // Address Byte 2
    msg[4] = (uint8_t)((addr)     & 0xFF);                      // Address Byte 1

    if(len < 9)
        return;

    // Request = Number of Bytes, Response = Buffer Side on Receiver side (bigger messages get rejected during transfer)
    msg[5] = (uint8_t)((bytes_size>>24) & 0xFF);            // Size Byte 4
    msg[6] = (uint8_t)((bytes_size>>16) & 0xFF);            // Size Byte 3
    msg[7] = (uint8_t)((bytes_size>>8)  & 0xFF);            // Size Byte 2
    msg[8] = (uint8_t)((bytes_size)     & 0xFF);            // Size Byte 1
}

// Request Download (0x34)
uint8_t *_create_request_download(int *len, uint8_t response, uint32_t addr, uint32_t bytes_size){
    uint8_t *msg = prepare_message(len, response, FBL_REQUEST_DOWNLOAD, 0, 9);
    if (msg == NULL)
        return msg;
    upload_download_message(msg, *len, addr, response, bytes_size);

    return msg;
}

// Request Upload (0x35)
uint8_t *_create_request_upload(int *len, uint8_t response, uint32_t addr, uint32_t bytes_size){
    uint8_t *msg = prepare_message(len, response, FBL_REQUEST_UPLOAD, 0, 9);
    if (msg == NULL)
        return msg;

    upload_download_message(msg, *len, addr, response, bytes_size);
    return msg;
}

// Transfer Data (0x36)
uint8_t *_create_transfer_data(int *len, uint8_t response, uint32_t addr, uint8_t* data, uint32_t data_len){
    if (data_len > (UINT32_MAX - 5)){
        *len = 0;
        return NULL;
    }

    uint8_t *msg = prepare_message(len, response, FBL_TRANSFER_DATA, 0, 5 + data_len);
    if (msg == NULL)
        return msg;

    upload_download_message(msg, *len, addr, 1, 0);
    for(uint32_t i = 0; i < data_len; i++)
        msg[5+i] = data[i];                                     // Payload
    return msg;
}


// Request Transfer Exit (0x37)
uint8_t *_create_request_transfer_exit(int *len, uint8_t response, uint32_t addr){
    uint8_t *msg = prepare_message(len, response, FBL_REQUEST_TRANSFER_EXIT, 0, 5);
    if (msg == NULL)
        return msg;

    msg[1] = (uint8_t)((addr>>24) & 0xFF);                      // Address Byte 4
    msg[2] = (uint8_t)((addr>>16) & 0xFF);                      // Address Byte 3
    msg[3] = (uint8_t)((addr>>8)  & 0xFF);                      // Address Byte 2
    msg[4] = (uint8_t)((addr)     & 0xFF);                      // Address Byte 1

    return msg;
}


//////////////////////////////////////////////////////////////////////////////
// Supported Common Response Codes
//////////////////////////////////////////////////////////////////////////////


// Negative Response (0x7F)
uint8_t *_create_neg_response(int *len, uint8_t rej_sid, uint8_t neg_resp_code) {
    uint8_t *msg = prepare_message(len, 0, FBL_NEGATIVE_RESPONSE, rej_sid, 3);
    if (msg == NULL)
        return msg;

    msg[2] = neg_resp_code;                                     // Negative Response Code
    return msg;
}


#ifdef __cplusplus
}
#endif   // _cplusplus
