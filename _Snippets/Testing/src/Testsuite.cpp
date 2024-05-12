// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : Testsuite.cpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Testsuite to verify communication for MCU and GUI
//============================================================================

#if defined(_Windows) || defined(_MSC_VER) || defined (__GNUC__)
 #define  STRICT
 #include <windows.h>
#endif

#include <stdio.h>
#include "Communication/uds_comm_spec.h"

void print_msg(uint8_t *msg, int len){
	printf("> Raw Data: ");
	for(int i = 0; i < len; i++){
		printf("%02X ", msg[i]);
	}
	printf("\n");
}

void print_msg_isotp(uint8_t *msg, int len){
	printf("> ISO-TP: ");
	for(int i = 0; i < len; i++){
		printf("%02X ", msg[i]);
	}
	printf("\n");
}

void iso_tp_frames(uint8_t *msg, int len){

	uint8_t *send_msg;
	int send_len;
	int has_next;
	int max_len_per_frame = 8; // CAN
	uint32_t data_ptr = 0;
	uint8_t idx = 0;

	send_msg = starting_frame(&send_len, &has_next, max_len_per_frame, msg, len, &data_ptr);
	print_msg_isotp(send_msg, send_len);
	free(send_msg);

	while(has_next){
		send_msg = consecutive_frame(&send_len, &has_next, max_len_per_frame, msg, len, &data_ptr, &idx);
		print_msg_isotp(send_msg, send_len);
		free(send_msg);
	}
}

int main() {

	uint8_t *msg;
	int len;

	printf("#########################################################\n");
	printf("Specification for Diagnostic and Communication Management\n");
	printf("#########################################################\n");

	printf("\n");

	printf("---------------------------------------------------------\n");
	printf("Diagnostic Session Control (SID 0x10)\n");
	printf("---------------------------------------------------------\n");

	printf("\n");

	printf("Diagnostic Session Control - Default Session - Request\n");
	msg = _create_diagnostic_session_control(&len, 0, FBL_DIAG_SESSION_DEFAULT);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("Diagnostic Session Control - Default Session - Response\n");
	msg = _create_diagnostic_session_control(&len, 1, FBL_DIAG_SESSION_DEFAULT);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("Diagnostic Session Control - Programming Session - Request\n");
	msg = _create_diagnostic_session_control(&len, 0, FBL_DIAG_SESSION_PROGRAMMING);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("Diagnostic Session Control - Programming Session - Response\n");
	msg = _create_diagnostic_session_control(&len, 1, FBL_DIAG_SESSION_PROGRAMMING);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("Diagnostic Session Control - Wrong/Unavailable Session - Request\n");
	msg = _create_diagnostic_session_control(&len, 0, 0x4);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("Negative Response - Diagnostic Session Control\n");
	msg = _create_neg_response(&len, FBL_DIAGNOSTIC_SESSION_CONTROL, FBL_RC_REQUEST_OUT_OF_RANGE);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("---------------------------------------------------------\n");
	printf("ECU Reset (SID 0x11)\n");
	printf("---------------------------------------------------------\n");

	printf("\n");

	printf("ECU Reset - PowerOn Reset - Request\n");
	msg = _create_ecu_reset(&len, 0, FBL_ECU_RESET_POWERON);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("ECU Reset - PowerOn Reset - Response\n");
	msg = _create_ecu_reset(&len, 1, FBL_ECU_RESET_POWERON);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("ECU Reset - Cold PowerOn Reset - Request\n");
	msg = _create_ecu_reset(&len, 0, FBL_ECU_RESET_COLD_POWERON);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("ECU Reset - Cold PowerOn Reset - Response\n");
	msg = _create_ecu_reset(&len, 1, FBL_ECU_RESET_COLD_POWERON);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("ECU Reset - Warm PowerOn Reset - Request\n");
	msg = _create_ecu_reset(&len, 0, FBL_ECU_RESET_WARM_POWERON);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("ECU Reset - Warm PowerOn Reset - Response\n");
	msg = _create_ecu_reset(&len, 1, FBL_ECU_RESET_WARM_POWERON);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("---------------------------------------------------------\n");
	printf("Security Access (SID 0x27)\n");
	printf("---------------------------------------------------------\n");

	printf("\n");

	printf("Security Access - Request SEED - Request\n");
	msg = _create_security_access(&len, 0, FBL_SEC_ACCESS_SEED, 0, 0);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("Security Access - Request SEED - Response\n");
	uint8_t key[7] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78};
	msg = _create_security_access(&len, 1, FBL_SEC_ACCESS_SEED, key, sizeof(key));
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("Security Access - Verify Key + Access Granted - Request\n");
	uint8_t verify_key[7] = {0x78, 0x67, 0x56, 0x45, 0x34, 0x23, 0x12};
	msg = _create_security_access(&len, 0, FBL_SEC_ACCESS_VERIFY_KEY, verify_key, sizeof(verify_key));
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("Security Access - Verify Key + Access Granted - Response\n");
	msg = _create_security_access(&len, 1, FBL_SEC_ACCESS_VERIFY_KEY, 0, 0);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("---------------------------------------------------------\n");
	printf("Tester Present (0x3E)\n");
	printf("---------------------------------------------------------\n");

	printf("\n");

	printf("Tester Present - With Response - Request\n");
	msg = _create_tester_present(&len, 0, FBL_TESTER_PRES_WITH_RESPONSE);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("Tester Present - With Response - Response\n");
	msg = _create_tester_present(&len, 1, FBL_TESTER_PRES_WITH_RESPONSE);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("Tester Present - Without Response - Request\n");
	msg = _create_tester_present(&len, 0, FBL_TESTER_PRES_WITHOUT_RESPONSE);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("#########################################################\n");
	printf("Specification for Data Transmission\n");
	printf("#########################################################\n");

	printf("\n");

	printf("---------------------------------------------------------\n");
	printf("Read Data By Identifier (SID 0x22)\n");
	printf("---------------------------------------------------------\n");

	printf("\n");

	printf("Read Data By Identifier - DID System Name - Request\n");
	msg = _create_read_data_by_ident(&len, 0, FBL_DID_SYSTEM_NAME, 0, 0);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("Read Data By Identifier - DID System Name - Response\n");
	uint8_t data[] = "AMOS FBL 24";
	msg = _create_read_data_by_ident(&len, 1, FBL_DID_SYSTEM_NAME, data, sizeof(data));
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("Read Data By Identifier - DID Programming Date - Request\n");
	msg = _create_read_data_by_ident(&len, 0, FBL_DID_PROGRAMMING_DATE, 0, 0);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("Read Data By Identifier - DID Programming Date - Response\n");
	uint8_t programming_date[] = {0x24, 0x05, 0x09};
	msg = _create_read_data_by_ident(&len, 1, FBL_DID_PROGRAMMING_DATE, programming_date, sizeof(programming_date));
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("---------------------------------------------------------\n");
	printf("Read Memory By Address (SID 0x23)\n");
	printf("---------------------------------------------------------\n");

	printf("\n");

	printf("Read Memory By Address - Adress 0xA0090000 - Request\n");
	msg = _create_read_memory_by_address(&len, 0, 0xA0090000, 1, 0, 0);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("Read Memory By Address - Adress 0xA0090000 - Request\n");
	uint8_t read_memory[] = {0xAB};
	msg = _create_read_memory_by_address(&len, 1, 0xA0090000, 1, read_memory, sizeof(read_memory));
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("---------------------------------------------------------\n");
	printf("Write Data By Identifier (SID 0x2E)\n");
	printf("---------------------------------------------------------\n");

	printf("\n");

	printf("Write Data By Identifier - DID System Name - Request\n");
	uint8_t write_data[] = "AMOS FBL 24. Let's test with a really long string that needs several loops. The idx for the different loops need to reach the value 15 and then start from 1 again!!";
	msg = _create_write_data_by_ident(&len, 0, FBL_DID_SYSTEM_NAME, write_data, sizeof(write_data));
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("Write Data By Identifier - DID System Name - Response\n");
	msg = _create_write_data_by_ident(&len, 1, FBL_DID_SYSTEM_NAME, 0, 0);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("#########################################################\n");
	printf("Specification for Upload | Download\n");
	printf("#########################################################\n");

	printf("\n");

	printf("Flow Control Frame - Response 1\n");
	msg = flow_control_frame(&len, 0, 0x10, 0x7F, 0);
	print_msg(msg, len);
	free(msg);

	printf("\n");

	printf("Flow Control Frame - Response 2\n");
	msg = flow_control_frame(&len, 2, 0x00, 0x7F, 3);
	print_msg(msg, len);
	free(msg);

	printf("\n");

	printf("Request Download - 0xA0090000 - Request\n");
	msg = _create_request_download(&len, 0, 0xA0090000, 0x00000005);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("Request Download - 0xA0090000 - Response\n");
	msg = _create_request_download(&len, 1, 0xA0090000, 0x00000005);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("Request Upload - 0xA0090000 - Request\n");
	msg = _create_request_upload(&len, 0, 0xA0090000, 0x00000005);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("Request Upload - 0xA0090000 - Response\n");
	msg = _create_request_upload(&len, 1, 0xA0090000, 0x00000005);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("Transfer Data - 0xA0090000 - Request\n");
	uint8_t transfer_data[] = {0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5};
	msg = _create_transfer_data(&len, 0xA0090000, transfer_data, (uint32_t)sizeof(transfer_data));
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	printf("Request Transfer Exit - 0xA0090000 - Request\n");
	msg = _create_request_transfer_exit(&len, 0, 0xA0090000);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("Request Transfer Exit - 0xA0090000 - Response\n");
	msg = _create_request_transfer_exit(&len, 1, 0xA01FFFFF);
	print_msg(msg, len);
	iso_tp_frames(msg, len);
	free(msg);

	printf("\n");

	return 0;
}
