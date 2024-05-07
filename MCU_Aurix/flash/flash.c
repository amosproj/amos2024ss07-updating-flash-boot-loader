// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>

//============================================================================
// Name        : flash.c
// Author      : Dorothea Ehrl
// Version     : 0.1
// Copyright   : MIT
// Description : Flash wrapper for Bootloader
//============================================================================

#include <string.h>

#include "IfxCpu.h"

#include "flash/flash.h"

#define PMU_FLASH_MODULE                0                           /* Macro to select the flash (PMU) module           */

/* Reserved space for erase and program routines in bytes */
#define ERASESECTOR_LEN             (100)
#define WAITUNBUSY_LEN              (100)
#define ENTERPAGEMODE_LEN           (100)
#define LOADPAGE2X32_LEN            (100)
#define WRITEPAGE_LEN               (100)
#define ERASEPFLASH_LEN             (0x100)
#define WRITEPFLASH_LEN             (0x200)

/* Definition of the addresses where to relocate the erase and program routines, given their reserved space */
#define ERASESECTOR_ADDR            (RELOCATION_START_ADDR)
#define WAITUNBUSY_ADDR             (ERASESECTOR_ADDR + ERASESECTOR_LEN)
#define ENTERPAGEMODE_ADDR          (WAITUNBUSY_ADDR + WAITUNBUSY_LEN)
#define LOAD2X32_ADDR               (ENTERPAGEMODE_ADDR + ENTERPAGEMODE_LEN)
#define WRITEPAGE_ADDR              (LOAD2X32_ADDR + LOADPAGE2X32_LEN)
#define ERASEPFLASH_ADDR            (WRITEPAGE_ADDR + WRITEPAGE_LEN)
#define WRITEPFLASH_ADDR            (ERASEPFLASH_ADDR + ERASEPFLASH_LEN)

#define MEM(address)                *((uint32 *)(address))      /* Macro to simplify the access to a memory address */

typedef struct
{
    void (*eraseSectors)(uint32 sectorAddr, uint32 numSector);
    uint8 (*waitUnbusy)(uint32 flash, IfxFlash_FlashType flashModule);
    uint8 (*enterPageMode)(uint32 pageAddr);
    void (*load2X32bits)(uint32 pageAddr, uint32 wordL, uint32 wordU);
    void (*writePage)(uint32 pageAddr);
    void (*erasePFlash)(IfxFlash_FlashType flashModule, uint32 sectorAddr, uint32 numSectors);
    void (*writePFlash)(IfxFlash_FlashType flashModule, uint32 startingAddr, uint32 numPages, uint32 data[], size_t dataSize);
} Function;

Function g_functionsFromPSPR;

/* Function to control LEDs to signal successful flashing.*/
void turn_led_on(Ifx_P *port, uint8 pinIndex)
{
    IfxPort_setPinLow(port, pinIndex);
}

void turn_led_off(Ifx_P *port, uint8 pinIndex)
{
    IfxPort_setPinHigh(port, pinIndex);
}

void init_leds(void)
{
    /* Configure LED1 and LED2 port pins */
    IfxPort_setPinMode(LED1, IfxPort_Mode_outputPushPullGeneral);
    IfxPort_setPinMode(LED2, IfxPort_Mode_outputPushPullGeneral);

    turn_led_off(LED2);
    turn_led_off(LED1);
}

/* This function erases a given sector of the Program Flash memory. The function is copied in the PSPR through
 * copyFunctionsToPSPR(). Because of this, inside the function, only routines from the PSPR or inline functions
 * can be called, otherwise a Context Type (CTYP) trap can be triggered.
 */
void erasePFlash(IfxFlash_FlashType flashModule, uint32 sectorAddr, uint32 numSectors)
{
    /* Get the current password of the Safety WatchDog module */
    uint16 endInitSafetyPassword = IfxScuWdt_getSafetyWatchdogPasswordInline();

    /* Erase the sector */
    IfxScuWdt_clearSafetyEndinitInline(endInitSafetyPassword);      /* Disable EndInit protection                   */
    g_functionsFromPSPR.eraseSectors(sectorAddr, numSectors);       /* Erase the given sector                       */
    IfxScuWdt_setSafetyEndinitInline(endInitSafetyPassword);        /* Enable EndInit protection                    */

    /* Wait until the sector is erased */
    g_functionsFromPSPR.waitUnbusy(PMU_FLASH_MODULE, flashModule);
}

/* This function writes the Program Flash memory. The function is copied in the PSPR through copyFunctionsToPSPR().
 * Because of this, inside the function, only routines from the PSPR or inline functions can be called,
 * otherwise a Context Type (CTYP) trap can be triggered.
 */
void writePFlash(IfxFlash_FlashType flashModule, uint32 startingAddr, uint32 numPages, uint32 data[], size_t dataSize)
{
    uint32 page;                                                /* Variable to cycle over all the pages             */
    uint32 offset;                                              /* Variable to cycle over all the words in a page   */

    /* Get the current password of the Safety WatchDog module */
    uint16 endInitSafetyPassword = IfxScuWdt_getSafetyWatchdogPasswordInline();
    uint32 index = 0;

    /* Write all the pages */
    for(page = 0; page < numPages; page++)              /* Loop over all the pages                  */
    {
        uint32 pageAddr = startingAddr + (page * PFLASH_PAGE_LENGTH);   /* Get the address of the page              */
        uint32* data_for_page = (uint32*) (((uint8*) data) + (page * PFLASH_PAGE_LENGTH));

        /* Enter in page mode */
        g_functionsFromPSPR.enterPageMode(pageAddr);

        /* Wait until page mode is entered */
        g_functionsFromPSPR.waitUnbusy(PMU_FLASH_MODULE, PROGRAM_FLASH_0);

        /* Write 32 bytes (8 double words) into the assembly buffer */
        for(offset = 0; (offset * sizeof(uint32)) < PFLASH_PAGE_LENGTH; offset += 2)     /* Loop over the page length                */
        {
            g_functionsFromPSPR.load2X32bits(pageAddr, data_for_page[offset], data_for_page[offset + 1]); /* Load 2 words of 32 bits each */
            index += 2;
            if (index >= dataSize)
            {
                break;
            }
        }

        /* Write the page */
        IfxScuWdt_clearSafetyEndinitInline(endInitSafetyPassword);      /* Disable EndInit protection               */
        g_functionsFromPSPR.writePage(pageAddr);                        /* Write the page                           */
        IfxScuWdt_setSafetyEndinitInline(endInitSafetyPassword);        /* Enable EndInit protection                */

        /* Wait until the page is written in the Program Flash memory */
        g_functionsFromPSPR.waitUnbusy(PMU_FLASH_MODULE, PROGRAM_FLASH_0);
    }
}

/* This function copies the erase and program routines to the Program Scratch-Pad SRAM (PSPR) of the CPU0 and assigns
 * function pointers to them.
 */
static void copyFunctionsToPSPR(void)
{
    /* Copy the IfxFlash_eraseMultipleSectors() routine and assign it to a function pointer */
    memcpy((void *)ERASESECTOR_ADDR, (const void *)IfxFlash_eraseMultipleSectors, ERASESECTOR_LEN);
    g_functionsFromPSPR.eraseSectors = (void *)ERASESECTOR_ADDR;

    /* Copy the IfxFlash_waitUnbusy() routine and assign it to a function pointer */
    memcpy((void *)WAITUNBUSY_ADDR, (const void *)IfxFlash_waitUnbusy, WAITUNBUSY_LEN);
    g_functionsFromPSPR.waitUnbusy = (void *)WAITUNBUSY_ADDR;

    /* Copy the IfxFlash_enterPageMode() routine and assign it to a function pointer */
    memcpy((void *)ENTERPAGEMODE_ADDR, (const void *)IfxFlash_enterPageMode, ENTERPAGEMODE_LEN);
    g_functionsFromPSPR.enterPageMode = (void *)ENTERPAGEMODE_ADDR;

    /* Copy the IfxFlash_loadPage2X32() routine and assign it to a function pointer */
    memcpy((void *)LOAD2X32_ADDR, (const void *)IfxFlash_loadPage2X32, LOADPAGE2X32_LEN);
    g_functionsFromPSPR.load2X32bits = (void *)LOAD2X32_ADDR;

    /* Copy the IfxFlash_writePage() routine and assign it to a function pointer */
    memcpy((void *)WRITEPAGE_ADDR, (const void *)IfxFlash_writePage, WRITEPAGE_LEN);
    g_functionsFromPSPR.writePage = (void *)WRITEPAGE_ADDR;

    /* Copy the erasePFLASH() routine and assign it to a function pointer */
    memcpy((void *)ERASEPFLASH_ADDR, (const void *)erasePFlash, ERASEPFLASH_LEN);
    g_functionsFromPSPR.erasePFlash = (void *)ERASEPFLASH_ADDR;

    /* Copy the erasePFLASH() routine and assign it to a function pointer */
    memcpy((void *)WRITEPFLASH_ADDR, (const void *)writePFlash, WRITEPFLASH_LEN);
    g_functionsFromPSPR.writePFlash = (void *)WRITEPFLASH_ADDR;
}

static uint32 getNumPerSize(size_t sectionLength, size_t dataSize)
{
    // dataSize is in uint32 values, but bytes needed
    dataSize *= sizeof(uint32);
    uint32 num_pages = dataSize / sectionLength;
    if (dataSize % sectionLength) // we need a page more because there is data left that does not fill a full page
    {
        num_pages++;
    }
    return num_pages;
}

static uint32 getDFlashNumPages(size_t dataSize)
{
    return getNumPerSize(DFLASH_PAGE_LENGTH, dataSize);
}

static uint32 getDFlashNumSectors(size_t dataSize)
{
    return getNumPerSize(DFLASH_SECTOR_LENGTH, dataSize);
}

static uint32 getPFlashNumPages(size_t dataSize)
{
    return getNumPerSize(PFLASH_PAGE_LENGTH, dataSize);
}

static uint32 getPFlashNumSectors(size_t dataSize)
{
    return getNumPerSize(PFLASH_SECTOR_LENGTH, dataSize);
}

static int checkAddrInFlashModule(IfxFlash_FlashType flashModule, uint32 addr)
{
    if(flashModule == DATA_FLASH_0)
    {
        if (addr >= DATA_FLASH_0_BASE_ADDR && addr < DATA_FLASH_0_END_ADDR)
        {
            return 0;
        }
    }
    else if(flashModule == DATA_FLASH_1)
    {
        if (addr >= DATA_FLASH_1_BASE_ADDR && addr < DATA_FLASH_1_END_ADDR)
        {
            return 0;
        }
    }
    else if(flashModule == PROGRAM_FLASH_0)
    {
        if (addr >= PROGRAM_FLASH_0_BASE_ADDR && addr < PROGRAM_FLASH_0_END_ADDR)
        {
            return 0;
        }
    }
    else if(flashModule == PROGRAM_FLASH_1)
    {
        if (addr >= PROGRAM_FLASH_1_BASE_ADDR && addr < PROGRAM_FLASH_1_END_ADDR)
        {
            return 0;
        }
    }
    return -1;
}

/* This function flashes the Program Flash memory calling the routines from the PSPR */
int writeProgramFlash(IfxFlash_FlashType flashModule, uint32 flashStartAddr, uint32 data[], size_t dataSize)
{
    if (flashModule != PROGRAM_FLASH_0 && flashModule != PROGRAM_FLASH_1)
    {
        return -1;
    }

    // check if address matches flashModule
    if (checkAddrInFlashModule(flashModule, flashStartAddr))
    {
        return -1;
    }

    // check if address range can be in flash
    if (checkAddrInFlashModule(flashModule, flashStartAddr + dataSize * sizeof(uint32)))
    {
        return -1;
    }

    uint32 num_sectors = getPFlashNumSectors(dataSize);
    uint32 num_pages = getPFlashNumPages(dataSize);

    boolean interruptState = IfxCpu_disableInterrupts(); /* Get the current state of the interrupts and disable them*/

    /* Copy all the needed functions to the PSPR memory to avoid overwriting them during the flash execution */
    copyFunctionsToPSPR();

    /* Erase the Program Flash sector before writing */
    g_functionsFromPSPR.erasePFlash(flashModule, flashStartAddr, num_sectors);

    /* Write the Program Flash */
    g_functionsFromPSPR.writePFlash(flashModule, flashStartAddr, num_pages, data, dataSize);

    IfxCpu_restoreInterrupts(interruptState);            /* Restore the interrupts state                            */
    return 0;
}

/* This function verifies if the data has been correctly written in the Program Flash */
uint32 verifyProgramFlash(uint32 flashStartAddr, uint32 data[], size_t dataSize)
{
    uint32 num_pages = getPFlashNumPages(dataSize);

    uint32 page;                                                /* Variable to cycle over all the pages             */
    uint32 offset;                                              /* Variable to cycle over all the words in a page   */
    uint32 errors = 0;                                          /* Variable to keep record of the errors            */
    uint32 index = 0;

    /* Verify the written data */
    for(page = 0; page < num_pages; page++)                          /* Loop over all the pages      */
    {
        uint32 pageAddr = flashStartAddr + (page * PFLASH_PAGE_LENGTH);    /* Get the address of the page  */

        for(offset = 0; offset < PFLASH_PAGE_LENGTH; offset += sizeof(uint32))                 /* Loop over the page length    */
        {
            /* Check if the data in the Program Flash is correct */
            if(MEM(pageAddr + offset) != data[index])
            {
                /* If not, count the found errors */
                errors++;
            }
            index++;
            if (index >= dataSize)
            {
                return errors;
            }
        }
    }
    return errors;
}

/* This function flashes the Data Flash memory.
 * It is not needed to run this function from the PSPR, thus functions from the Program Flash memory can be called
 * inside.
 */
int writeDataFlash(IfxFlash_FlashType flashModule, uint32 flashStartAddr, uint32 data[], size_t dataSize)
{
    if (flashModule != DATA_FLASH_0 && flashModule != DATA_FLASH_1)
    {
        return -1;
    }

    // check if address matches flashModule
    if (checkAddrInFlashModule(flashModule, flashStartAddr))
    {
        return -1;
    }

    // check if address range can be in flash
    if (checkAddrInFlashModule(flashModule, flashStartAddr + dataSize))
    {
        return -1;
    }

    uint32 num_sectors = getDFlashNumSectors(dataSize);
    uint32 num_pages = getDFlashNumPages(dataSize);

    uint32 page;                                                /* Variable to cycle over all the pages             */

    /* --------------- ERASE PROCESS --------------- */
    /* Get the current password of the Safety WatchDog module */
    uint16 endInitSafetyPassword = IfxScuWdt_getSafetyWatchdogPassword();

    /* Erase the sector */
    IfxScuWdt_clearSafetyEndinit(endInitSafetyPassword);        /* Disable EndInit protection                       */
    IfxFlash_eraseMultipleSectors(flashStartAddr, num_sectors); /* Erase the given sector           */
    IfxScuWdt_setSafetyEndinit(endInitSafetyPassword);          /* Enable EndInit protection                        */

    /* Wait until the sector is erased */
    IfxFlash_waitUnbusy(flashModule, DATA_FLASH_0);

    /* --------------- WRITE PROCESS --------------- */
    for(page = 0; page < num_pages; page++)      /* Loop over all the pages                          */
    {
        uint32 page_addr = flashStartAddr + (page * DFLASH_PAGE_LENGTH); /* Get the address of the page     */
        uint32* data_for_page = (uint32*) (((uint8*) data) + (page * DFLASH_PAGE_LENGTH));

        /* Enter in page mode */
        IfxFlash_enterPageMode(page_addr);

        /* Wait until page mode is entered */
        IfxFlash_waitUnbusy(flashModule, DATA_FLASH_0);

        /* Load data to be written in the page */
        IfxFlash_loadPage2X32(page_addr, data_for_page[0], data_for_page[1]); /* Load two words of 32 bits each            */

        /* Write the loaded page */
        IfxScuWdt_clearSafetyEndinit(endInitSafetyPassword);    /* Disable EndInit protection                       */
        IfxFlash_writePage(page_addr);                           /* Write the page                                   */
        IfxScuWdt_setSafetyEndinit(endInitSafetyPassword);      /* Enable EndInit protection                        */

        /* Wait until the data is written in the Data Flash memory */
        IfxFlash_waitUnbusy(flashModule, DATA_FLASH_0);
    }
    return 0;
}

/* This function verifies if the data has been correctly written in the Data Flash */
uint32 verifyDataFlash(uint32 flashStartAddress, uint32 data[], size_t dataSize)
{
    uint32 num_pages = getDFlashNumPages(dataSize);

    uint32 page;                                                /* Variable to cycle over all the pages             */
    uint32 offset;                                              /* Variable to cycle over all the words in a page   */
    uint32 errors = 0;                                          /* Variable to keep record of the errors            */
    uint32 index = 0;

    /* Verify the written data */
    for(page = 0; page < num_pages; page++)                          /* Loop over all the pages      */
    {
        uint32 page_addr = flashStartAddress + (page * DFLASH_PAGE_LENGTH);    /* Get the address of the page  */

        for(offset = 0; offset < DFLASH_PAGE_LENGTH; offset += sizeof(uint32))            /* Loop over the page length    */
        {
            /* Check if the data in the Data Flash is correct */
            if(MEM(page_addr + offset) != data[index])
            {
                /* If not, count the found errors */
                errors++;
            }
            index++;
            if (index >= dataSize)
            {
                return errors;
            }
        }
    }
    return errors;
}
