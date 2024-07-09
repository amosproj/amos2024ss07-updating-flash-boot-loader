// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>

//============================================================================
// Name        : flash_TC375.h
// Author      : Dorothea Ehrl
// Version     : 0.2
// Copyright   : MIT
// Description : Flash wrapper for Bootloader
//============================================================================

#ifndef FLASH_TC375_LK_H_
#define FLASH_TC375_LK_H_

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/

#include "Ifx_Types.h"
#include "IfxFlash.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/

/* Define the Program Flash Bank to be used.*/
#define PROGRAM_FLASH_0             IfxFlash_FlashType_P0
#define PROGRAM_FLASH_1             IfxFlash_FlashType_P1
/* Define the Data Flash Bank to be used.*/
#define DATA_FLASH_0                IfxFlash_FlashType_D0
#define DATA_FLASH_1                IfxFlash_FlashType_D1

/* Important for writing into PFlash and DFlash - Take care about borders for erasing */
#define PFLASH_SECTOR_LENGTH        0x4000                       /* 16KB - logical sector length */
#define PFLASH_PHY_SECTOR_LENGTH    0x100000                     /* 1MB - real physical sector length, here reduced to 16KB blocks */
#define DFLASH_SECTOR_LENGTH        0xFFF                        /* 4KB - default sector length in single ended mode */


/* Relocation address for the erase and program routines: Program Scratch-Pad SRAM (PSPR) of CPU0 */
#define PSPR_START_ADDR             (0x70100000U)

#define DATA_FLASH_0_BASE_ADDR      0xAF000000
#define DATA_FLASH_0_END_ADDR       0xAF0FFFFF

#define DATA_FLASH_1_BASE_ADDR      0xAFC00000
#define DATA_FLASH_1_END_ADDR       0xAFC1FFFF

// WRITEABLE ADDRESSES - Need to match Physical and Logical Sector sizes and start at a physical sector
/*
 * Default PFlash 0:
 *      Start   0xA0090000
 *      End     0xA02FFFFF
 *      Size    0x00270000 -> 2,4375 physical sectors (2 full physical + 28 logical sectors), starting at a physical sector
 */
#define PROGRAM_FLASH_0_PHY_BASE_ADDR   0xA0000000  // Real Start Address (used for calculation)
#define PROGRAM_FLASH_0_PHY_END_ADDR    0xA02FFFFF  // Real End Address
#define PROGRAM_FLASH_0_BASE_ADDR       0xA0090000  // Writeable Start Address (hard coded boundary to avoid overwriting reserved areas)
#define PROGRAM_FLASH_0_END_ADDR        0xA02FFFFF  // Writeable End Address (hard coded boundary to avoid overwriting reserved areas)

/*
 * Default PFlash 1:
 *      Start   0xA0300000
 *      End     0xA05FFFFF
 *      Size    0x00300000 -> 3 full physical sectors, starting at a physical sector
 */
#define PROGRAM_FLASH_1_PHY_BASE_ADDR   0xA0300000  // Real Start Address (used for calculation)
#define PROGRAM_FLASH_1_PHY_END_ADDR    0xA05FFFFF  // Real End Address
#define PROGRAM_FLASH_1_BASE_ADDR       0xA0300000  // Writeable Start Address (hard coded boundary to avoid overwriting reserved areas)
#define PROGRAM_FLASH_1_END_ADDR        0xA05FFFFF  // Writeable End Address (hard coded boundary to avoid overwriting reserved areas)

#define PFLASH_PAGE_LENGTH          IFXFLASH_PFLASH_PAGE_LENGTH /* 0x20 = 32 Bytes (smallest unit that can be
                                                                 * programmed in the Program Flash memory (PFLASH)) */
#define PFLASH_LAST_PAGE_SIZE       (PFLASH_PAGE_LENGTH / 4)    /* 32 byte for 8 double words (uint32_t) */
#define DFLASH_PAGE_LENGTH          IFXFLASH_DFLASH_PAGE_LENGTH /* 0x8 = 8 Bytes (smallest unit that can be
                                                                 * programmed in the Data Flash memory (DFLASH))    */

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
