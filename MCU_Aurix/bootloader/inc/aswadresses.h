// SPDX-License-Identifier: MIT

// SPDX-FileCopyrightText: 2024 Sebastian Rodriguez <r99@melao.de>

//============================================================================
// Name        : aswadresses.h
// Author      : Sebastian Rodriguez
// Version     : 0.1
// Copyright   : MIT
// Description : Memory Addresses for ASW
//============================================================================

#ifndef BOOTLOADER_INC_ASWADRESSES_H_
#define BOOTLOADER_INC_ASWADRESSES_H_

#define ASW0_TRAPVEC 0x80090100
#define ASW0_INTVEC 0x804F4000
#define ASW0_CORE0 0x80092130

/*TODO:
What is the ISTACK adress of ASW?
Do we have to chenge registers a8 and a9?
*/

#endif /* BOOTLOADER_INC_ASWADRESSES_H_ */
