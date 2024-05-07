// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>

//============================================================================
// Name        : flash_TC375.h
// Author      : Dorothea Ehrl
// Version     : 0.1
// Copyright   : MIT
// Description : Flash wrapper for Bootloader
//============================================================================

#ifndef FLASH_TC375_LK_H_
#define FLASH_TC375_LK_H_

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/

/* Definition of the LEDs port pins */
#define LED1                        &MODULE_P00,5               /* LED connected to Port 00, Pin 5                  */
#define LED2                        &MODULE_P00,6               /* LED connected to Port 00, Pin 6                  */

/* Define the Program Flash Bank to be used.*/
#define PROGRAM_FLASH_0             IfxFlash_FlashType_P0
#define PROGRAM_FLASH_1             IfxFlash_FlashType_P1
/* Define the Data Flash Bank to be used.*/
#define DATA_FLASH_0                IfxFlash_FlashType_D0
#define DATA_FLASH_1                IfxFlash_FlashType_D1

/* Relocation address for the erase and program routines: Program Scratch-Pad SRAM (PSPR) of CPU0 */
#define RELOCATION_START_ADDR       (0x70100000U) // TODO rename maybe PSPR_START_ADDR

#define DATA_FLASH_0_BASE_ADDR      0xAF000000
#define DATA_FLASH_0_END_ADDR       0xAF0FFFFF

#define DATA_FLASH_1_BASE_ADDR      0xAFC00000
#define DATA_FLASH_1_END_ADDR       0xAFC1FFFF

// TODO are these the correct addresses for PF?
#define PROGRAM_FLASH_0_BASE_ADDR   0xA00E0000 // TODO eig 0xA0000000
#define PROGRAM_FLASH_0_END_ADDR    0xA02FFFFF

#define PROGRAM_FLASH_1_BASE_ADDR   0xA0300000
#define PROGRAM_FLASH_1_END_ADDR    0xA05FFFFF

// not sure if page size is uC specific
#define PFLASH_PAGE_LENGTH          IFXFLASH_PFLASH_PAGE_LENGTH /* 0x20 = 32 Bytes (smallest unit that can be
                                                                 * programmed in the Program Flash memory (PFLASH)) */
#define DFLASH_PAGE_LENGTH          IFXFLASH_DFLASH_PAGE_LENGTH /* 0x8 = 8 Bytes (smallest unit that can be
                                                                 * programmed in the Data Flash memory (DFLASH))    */

// TODO kilo or kibi Byte
#define PFLASH_SECTOR_LENGTH        16000                       /* logical sector length */

#define DFLASH_SECTOR_LENGTH        4000                        /* default sector length in single ended mode */
/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*-------------------------------------------------Data Structures---------------------------------------------------*/
/*********************************************************************************************************************/
 
/*********************************************************************************************************************/
/*--------------------------------------------Private Variables/Constants--------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/


#endif /* FLASH_TC375_LK_H_ */
