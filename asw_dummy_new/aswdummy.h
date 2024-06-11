// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Sebastian Rodriguez <r99@melao.de>

//============================================================================
// Name        : aswdummy.h
// Author      : Sebastian Rodriguez
// Version     : 0.1
// Copyright   : MIT
// Description : Header file for the dummy ASW
//============================================================================

#ifndef ASWDUMMY_H_
#define ASWDUMMY_H_

#include <stdint.h>

typedef struct{
	uint64_t magic_number;
	char asw_version[8];
	char asw_version_comment[40]; // e.g. Bremse vorne links
	uint64_t magic_number2;
} version_info;

void alternating_blinking(void);

#endif /* ASWDUMMY_H_ */
