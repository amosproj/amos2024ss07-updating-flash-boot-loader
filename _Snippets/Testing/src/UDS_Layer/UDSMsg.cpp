// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : UDSMsg.cpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : UDS Message Class
//============================================================================

#include "UDSMsg.h"

#if defined(_Windows) || defined(_MSC_VER) || defined (__GNUC__)
 #define  STRICT
 #include <windows.h>
#endif

UDS_Msg::UDS_Msg(uint8_t *data, uint32_t len) {
	this->sid = sid;
	this->data = data;
	this->len = len;

}

UDS_Msg::~UDS_Msg() {
	free(data);
	len = 0;
}

uint8_t UDS_Msg::getSID(){
	if(len > 0)
		return data[0];
	else
		return 0;
}

uint8_t* UDS_Msg::getData(uint32_t *len){
	*len = this->len;
	return data;
}


