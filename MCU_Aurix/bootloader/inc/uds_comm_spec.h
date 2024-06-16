// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>
// SPDX-FileCopyrightText: 2024 Leon Wilms <leonwilms.wk@gmail.com>
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>

//============================================================================
// Name        : uds_comm_spec.h
// Author      : Michael Bauer, Leon Wilms, Wiktor Pilarczyk
// Version     : 1.0
// Copyright   : MIT
// Description : UDS communication specification for AMOS Flashbootloader
//============================================================================

#ifndef COMMUNICATION_UDS_COMM_SPEC_H_
#define COMMUNICATION_UDS_COMM_SPEC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

#define MAX_FRAME_LEN_CAN                                           (0x08)
#define MAX_FRAME_LEN_CANFD                                         (0x40)  // Used for single frame buffer (MCU)
#define MAX_ISOTP_MESSAGE_LEN                                       (4096)  // Used for starting frame + consecutive frames buffer (MCU)
#define FBLCAN_IDENTIFIER_MASK                                      (0x0F24FFFF)
#define FBLCAN_BASE_ADDRESS                                         (FBLCAN_IDENTIFIER_MASK & 0xFFFF0000)

//////////////////////////////////////////////////////////////////////////////
// Supported Service Overview (SID)
//////////////////////////////////////////////////////////////////////////////

/**
 * Diagnostic and Communication Management
 */
#define FBL_DIAGNOSTIC_SESSION_CONTROL                              (0x10)
#define FBL_ECU_RESET                                               (0x11)
#define FBL_SECURITY_ACCESS                                         (0x27)
#define FBL_TESTER_PRESENT                                          (0x3E)

/**
 * Data Transmission
 */
#define FBL_READ_DATA_BY_IDENTIFIER                                 (0x22)
#define FBL_READ_MEMORY_BY_ADDRESS                                  (0x23)
#define FBL_WRITE_DATA_BY_IDENTIFIER                                (0x2E)

/**
 * Upload | Download
 */
#define FBL_REQUEST_DOWNLOAD                                        (0x34)
#define FBL_REQUEST_UPLOAD                                          (0x35)
#define FBL_TRANSFER_DATA                                           (0x36)
#define FBL_REQUEST_TRANSFER_EXIT                                   (0x37)

/**
 * Negative Response
 */
#define FBL_NEGATIVE_RESPONSE                                       (0x7F)

//////////////////////////////////////////////////////////////////////////////
// Others regarding SID
//////////////////////////////////////////////////////////////////////////////

#define FBL_SID_ACK                                                 (0x40)

//////////////////////////////////////////////////////////////////////////////
// Supported Common Response Codes
//////////////////////////////////////////////////////////////////////////////

#define FBL_RC_GENERAL_REJECT                                       (0x10)
#define FBL_RC_SERVICE_NOT_SUPPORTED                                (0x11)
#define FBL_RC_SUB_FUNC_NOT_SUPPORTED                               (0x12)
#define FBL_RC_INCORRECT_MSG_LEN_OR_INV_FORMAT                      (0x13)
#define FBL_RC_RESPONSE_TOO_LONG                                    (0x14)
#define FBL_RC_BUSY_REPEAT_REQUEST                                  (0x21)
#define FBL_RC_CONDITIONS_NOT_CORRECT                               (0x22)
#define FBL_RC_REQUEST_SEQUENCE_ERROR                               (0x24)
#define FBL_RC_FAILURE_PREVENTS_EXEC_OF_REQUESTED_ACTION            (0x26)
#define FBL_RC_REQUEST_OUT_OF_RANGE                                 (0x31)
#define FBL_RC_SECURITY_ACCESS_DENIED                               (0x33)
#define FBL_RC_INVALID_KEY                                          (0x35)
#define FBL_RC_EXCEEDED_NUMBER_OF_ATTEMPTS                          (0x36)
#define FBL_RC_REQUIRED_TIME_DELAY_NOT_EXPIRED                      (0x37)
#define FBL_RC_UPLOAD_DOWNLOAD_NOT_ACCEPTED                         (0x70)
#define FBL_RC_TRANSFER_DATA_SUSPENDED                              (0x71)
#define FBL_RC_GENERAL_PROGRAMMING_FAILURE                          (0x72)
#define FBL_RC_WRONG_BLOCK_SEQUENCE_COUNTER                         (0x73)
#define FBL_RC_SUB_FUNC_NOT_SUPPORTED_IN_ACTIVE_SESSION             (0x7E)
#define FBL_RC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION              (0x7F)


//////////////////////////////////////////////////////////////////////////////
// Diagnostic Session Control (0x10)
//////////////////////////////////////////////////////////////////////////////
#define FBL_DIAG_SESSION_DEFAULT                                    (0x01)
#define FBL_DIAG_SESSION_PROGRAMMING                                (0x02)

//////////////////////////////////////////////////////////////////////////////
// ECU Reset (0x11)
//////////////////////////////////////////////////////////////////////////////
#define FBL_ECU_RESET_HARD                                          (0x01)
#define FBL_ECU_RESET_SOFT                                          (0x03)

//////////////////////////////////////////////////////////////////////////////
// Security Access (0x27)
//////////////////////////////////////////////////////////////////////////////
#define FBL_SEC_ACCESS_SEED                                         (0x01)
#define FBL_SEC_ACCESS_VERIFY_KEY                                   (0x02)

//////////////////////////////////////////////////////////////////////////////
// Tester Present (0x3E)
//////////////////////////////////////////////////////////////////////////////
#define FBL_TESTER_PRES_WITH_RESPONSE                               (0x01)
#define FBL_TESTER_PRES_WITHOUT_RESPONSE                            (0x02)

//////////////////////////////////////////////////////////////////////////////
// Read Data By Identifier (0x22)
//////////////////////////////////////////////////////////////////////////////
#define FBL_DID_SYSTEM_NAME                                         (0xF197)
#define FBL_DID_PROGRAMMING_DATE                                    (0xF199)
#define FBL_DID_BL_KEY_ADDRESS                                      (0xFD00)
#define FBL_DID_BL_KEY_GOOD_VALUE                                   (0xFD01)
#define FBL_DID_CAN_BASE_MASK                                       (0xFD02)
#define FBL_DID_CAN_ID                                              (0xFD03)
#define FBL_DID_BL_WRITE_START_ADD_CORE0                            (0xFD10)
#define FBL_DID_BL_WRITE_END_ADD_CORE0                              (0xFD11)
#define FBL_DID_BL_WRITE_START_ADD_CORE1                            (0xFD12)
#define FBL_DID_BL_WRITE_END_ADD_CORE1                              (0xFD13)
#define FBL_DID_BL_WRITE_START_ADD_CORE2                            (0xFD14)
#define FBL_DID_BL_WRITE_END_ADD_CORE2                              (0xFD15)

//############################################################################

//////////////////////////////////////////////////////////////////////////////
// ISO TP Handling
//////////////////////////////////////////////////////////////////////////////


uint8_t *tx_starting_frame(uint32_t *data_out_len, uint32_t *has_next, uint8_t max_len_per_frame, uint8_t* data_in, uint32_t data_in_len, uint32_t* data_out_idx_ctr);
uint8_t *tx_consecutive_frame(uint32_t *data_out_len, uint32_t *has_next, uint8_t max_len_per_frame, uint8_t* data_in, uint32_t data_in_len, uint32_t* data_out_idx_ctr, uint8_t* frame_idx);
uint8_t *tx_flow_control_frame(uint32_t *data_out_len, uint8_t flag, uint8_t blocksize, uint8_t sep_time_millis, uint8_t sep_time_multi_millis);

uint8_t rx_is_starting_frame(uint8_t* data_in, uint32_t data_in_len, uint8_t max_len_per_frame);
uint8_t rx_is_consecutive_frame(uint8_t* data_in, uint32_t data_in_len, uint8_t max_len_per_frame);
uint8_t rx_is_single_Frame(uint8_t* data_in, uint32_t data_in_len, uint8_t max_len_per_frame);
uint8_t rx_is_flow_control_frame(uint8_t* data_in, uint32_t data_in_len, uint8_t max_len_per_frame);
uint8_t *rx_starting_frame(uint32_t *data_out_len, uint32_t *has_next, uint8_t max_len_per_frame, uint8_t* data_in, uint32_t data_in_len);
uint8_t rx_consecutive_frame(uint32_t *data_out_len, uint8_t *data_out, uint32_t *has_next, uint32_t data_in_len, uint8_t* data_in, uint32_t *idx); // TODO: Error Handling for correct order

//////////////////////////////////////////////////////////////////////////////
// Templates for UDS Messages
//////////////////////////////////////////////////////////////////////////////

// Specification for Diagnostic and Communication Management
uint8_t *_create_diagnostic_session_control(int *len, uint8_t response, uint8_t session);
uint8_t *_create_ecu_reset(int *len, uint8_t response, uint8_t reset_type);
uint8_t *_create_security_access(int *len, uint8_t response, uint8_t request_type, uint8_t* key, uint8_t key_len);
uint8_t *_create_tester_present(int *len, uint8_t response, uint8_t response_type);


// Specification for Data Transmission
uint8_t *_create_read_data_by_ident(int *len, uint8_t response, uint16_t did, uint8_t* data, uint8_t data_len);
uint8_t *_create_read_memory_by_address(int *len, uint8_t response, uint32_t add, uint16_t no_bytes, uint8_t* data, uint16_t data_len);
uint8_t *_create_write_data_by_ident(int *len, uint8_t response, uint16_t did, uint8_t* data, uint8_t data_len);

// Specification for Upload | Download
uint8_t *_create_request_download(int *len, uint8_t response, uint32_t add, uint32_t bytes_size);
uint8_t *_create_request_upload(int *len, uint8_t response, uint32_t add, uint32_t bytes_size);
uint8_t *_create_transfer_data(int *len, uint8_t response, uint32_t add, uint8_t* data, uint32_t data_len);
uint8_t *_create_request_transfer_exit(int *len, uint8_t response, uint32_t add);

// Supported Common Response Codes
uint8_t *_create_neg_response(int *len, uint8_t rej_sid, uint8_t neg_resp_code);

#ifdef __cplusplus
}
#endif   // _cplusplus


#endif /* COMMUNICATION_UDS_COMM_SPEC_H_ */
