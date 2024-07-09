// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Dorothea Ehrl <dorothea.ehrl@fau.de>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : flash_driver.c
// Author      : Dorothea Ehrl, Michael Bauer, Paul Roy
// Version     : 0.3
// Copyright   : MIT
// Description : Flash wrapper for Bootloader
//============================================================================

#pragma optimize R

#include <string.h>
#include <stdlib.h> /* calloc, exit, free */
#include <stdio.h>

#include "IfxCpu.h"

#include "flash_driver.h"
#include "flash_driver_TC375_LK.h"
#include "crc.h"
#include "memory.h"
#include "uds_comm_spec.h"

#define PMU_FLASH_MODULE             0               /* Macro to select the flash (PMU) module           */

/* Reserved space for erase and program routines in bytes */
#define ERASESECTOR_LEN             (100)
#define WAITUNBUSY_LEN              (100)
#define ENTERPAGEMODE_LEN           (100)
#define LOADPAGE2X32_LEN            (100)
#define WRITEPAGE_LEN               (100)
#define ERASEPFLASH_LEN             (0x100)
#define WRITEPFLASH_LEN             (0x200)

/* Definition of the addresses where to relocate the erase and program routines, given their reserved space */
#define ERASESECTOR_ADDR            (PSPR_START_ADDR)
#define WAITUNBUSY_ADDR             (ERASESECTOR_ADDR + ERASESECTOR_LEN)
#define ENTERPAGEMODE_ADDR          (WAITUNBUSY_ADDR + WAITUNBUSY_LEN)
#define LOAD2X32_ADDR               (ENTERPAGEMODE_ADDR + ENTERPAGEMODE_LEN)
#define WRITEPAGE_ADDR              (LOAD2X32_ADDR + LOADPAGE2X32_LEN)
#define ERASEPFLASH_ADDR            (WRITEPAGE_ADDR + WRITEPAGE_LEN)
#define WRITEPFLASH_ADDR            (ERASEPFLASH_ADDR + ERASEPFLASH_LEN)

#define MEM(address)                *((uint32_t *)(address))      /* Macro to simplify the access to a memory address */

/*********************************************************************************************************************/
/*--------------------------------------------Private Variables/Constants--------------------------------------------*/
/*********************************************************************************************************************/


typedef struct
{
    void (*eraseSectors)(uint32 sectorAddr, uint32 numSector);
    uint8 (*waitUnbusy)(uint32 flash, IfxFlash_FlashType flashModule);
    uint8 (*enterPageMode)(uint32 pageAddr);
    void (*load2X32bits)(uint32 pageAddr, uint32 wordL, uint32 wordU);
    void (*writePage)(uint32 pageAddr);
    void (*erasePFlash)(IfxFlash_FlashType flashModule, uint32_t sectorAddr, uint32_t numSectors);
    void (*writePFlash)(IfxFlash_FlashType flashModule, uint32_t startingAddr, uint32_t numPages, uint32_t data[], size_t dataSize);
} Flash_Function;

Flash_Function g_functionsFromPSPR;

uint32_t flash_driver_last_flashpage[PFLASH_LAST_PAGE_SIZE];

typedef struct
{
        uint32_t init;
        uint32_t core0_start_addr;
        uint32_t core0_end_addr;
        uint32_t core0_erased_sections;
        uint32_t core1_start_addr;
        uint32_t core1_end_addr;
        uint32_t core1_erased_sections;
        uint32_t core2_start_addr;
        uint32_t core2_end_addr;
        uint32_t core2_erased_sections;
        uint32_t asw_key_start_addr;
        uint32_t asw_key_end_addr;
        uint32_t asw_key_erased_sections;
        uint32_t cal_data_start_addr;
        uint32_t cal_data_end_addr;
        uint32_t cal_data_erased_sections;
} Flash_Eraser;

Flash_Eraser pflash_eraser;

/*********************************************************************************************************************/
/*--------------------------------------------Private Helper Functions-----------------------------------------------*/
/*********************************************************************************************************************/

static uint32_t flashGetDIDData(uint16_t DID){
    uint32_t data = 0;

    uint8_t len, nrc;
    uint8_t *read = readData(DID, &len, &nrc);
    if(!nrc){
        if(len == 4){ // uint32_t
            data |= (read[0] << 24);
            data |= (read[1] << 16);
            data |= (read[2] << 8);
            data |= read[3];
        }
    }
    free(read);
    return data;
}

static void createLastFlashPage(uint32_t last_page_addr, uint32_t *data, size_t dataSize){

    /* Write 32 bytes (8 double words) into the last page buffer */
    for(uint32_t index = 0; index < PFLASH_LAST_PAGE_SIZE; index++)
    {
        // Use given data
        if(index < dataSize){
            flash_driver_last_flashpage[index] = *(data+index);
        }

        // fill the rest of the last page with zeros
        else {
            flash_driver_last_flashpage[index] = 0;
        }
    }
}

/* This function erases a given sector of the Program Flash memory. The function is copied in the PSPR through
 * copyFunctionsToPSPR(). Because of this, inside the function, only routines from the PSPR or inline functions
 * can be called, otherwise a Context Type (CTYP) trap can be triggered.
 */
static void erasePFlash(IfxFlash_FlashType flashModule, uint32_t sectorAddr, uint32_t numSectors)
{
    /* Get the current password of the Safety WatchDog module */
    uint16 endInitSafetyPassword = IfxScuWdt_getSafetyWatchdogPasswordInline();

    IfxScuWdt_clearSafetyEndinitInline(endInitSafetyPassword);      /* Disable EndInit protection                   */
    g_functionsFromPSPR.eraseSectors(sectorAddr, numSectors);
    IfxScuWdt_setSafetyEndinitInline(endInitSafetyPassword);        /* Enable EndInit protection                    */

    /* Wait until the sector is erased */
    g_functionsFromPSPR.waitUnbusy(PMU_FLASH_MODULE, flashModule);
}

/* This function writes the Program Flash memory. The function is copied in the PSPR through copyFunctionsToPSPR().
 * Because of this, inside the function, only routines from the PSPR or inline functions can be called,
 * otherwise a Context Type (CTYP) trap can be triggered.
 */
static void writePFlash(IfxFlash_FlashType flashModule, uint32_t startingAddr, uint32_t numPages, uint32_t data[], size_t dataSize)
{
    uint32_t page;
    uint32_t offset;

    /* Get the current password of the Safety WatchDog module */
    uint16 endInitSafetyPassword = IfxScuWdt_getSafetyWatchdogPasswordInline();
    uint32_t index = 0;

    for(page = 0; page < numPages; page++)
    {
        uint32_t pageAddr = startingAddr + (page * PFLASH_PAGE_LENGTH);
        uint32* data_for_page = (uint32*) (((uint8*) data) + (page * PFLASH_PAGE_LENGTH));

        g_functionsFromPSPR.enterPageMode(pageAddr); // enter page mode to be able to write in page

        /* Wait until page mode is entered */
        g_functionsFromPSPR.waitUnbusy(PMU_FLASH_MODULE, flashModule);

        index = 0;
        /* Write 32 bytes (8 double words) into the assembly buffer */
        for(offset = 0; (offset * sizeof(uint32)) < PFLASH_PAGE_LENGTH; offset += 2)
        {

            if(page < numPages-1)
                g_functionsFromPSPR.load2X32bits(pageAddr, data_for_page[offset], data_for_page[offset + 1]); // Load 2 words of 32 bits each
            else // Act different for last page
                g_functionsFromPSPR.load2X32bits(pageAddr, flash_driver_last_flashpage[index], flash_driver_last_flashpage[index + 1]); // Load 2 words of 32 bits each

            index += 2;
        }

        /* Write the page */
        IfxScuWdt_clearSafetyEndinitInline(endInitSafetyPassword);      /* Disable EndInit protection               */
        g_functionsFromPSPR.writePage(pageAddr);
        IfxScuWdt_setSafetyEndinitInline(endInitSafetyPassword);        /* Enable EndInit protection                */

        /* Wait until the page is written in the Program Flash memory */
        g_functionsFromPSPR.waitUnbusy(PMU_FLASH_MODULE, flashModule);
    }
}

/* This function copies the erase and program routines to the Program Scratch-Pad SRAM (PSPR) of the CPU0 and assigns
 * function pointers to them.
 */
static void copyFunctionsToPSPR(void)
{
    /* Copy multiple needed routines and assign it to function pointers */
    memcpy((void *)ERASESECTOR_ADDR, (const void *)IfxFlash_eraseMultipleSectors, ERASESECTOR_LEN);
    g_functionsFromPSPR.eraseSectors = (void *)ERASESECTOR_ADDR;

    memcpy((void *)WAITUNBUSY_ADDR, (const void *)IfxFlash_waitUnbusy, WAITUNBUSY_LEN);
    g_functionsFromPSPR.waitUnbusy = (void *)WAITUNBUSY_ADDR;

    memcpy((void *)ENTERPAGEMODE_ADDR, (const void *)IfxFlash_enterPageMode, ENTERPAGEMODE_LEN);
    g_functionsFromPSPR.enterPageMode = (void *)ENTERPAGEMODE_ADDR;

    memcpy((void *)LOAD2X32_ADDR, (const void *)IfxFlash_loadPage2X32, LOADPAGE2X32_LEN);
    g_functionsFromPSPR.load2X32bits = (void *)LOAD2X32_ADDR;

    memcpy((void *)WRITEPAGE_ADDR, (const void *)IfxFlash_writePage, WRITEPAGE_LEN);
    g_functionsFromPSPR.writePage = (void *)WRITEPAGE_ADDR;

    memcpy((void *)ERASEPFLASH_ADDR, (const void *)erasePFlash, ERASEPFLASH_LEN);
    g_functionsFromPSPR.erasePFlash = (void *)ERASEPFLASH_ADDR;

    memcpy((void *)WRITEPFLASH_ADDR, (const void *)writePFlash, WRITEPFLASH_LEN);
    g_functionsFromPSPR.writePFlash = (void *)WRITEPFLASH_ADDR;
}

static uint32_t getNumPerSize(size_t sectionLength, size_t dataSize, bool upscale_to_byte)
{
    if(upscale_to_byte)
        dataSize *= sizeof(uint32_t); // dataSize is in uint32_t values, but bytes needed
    uint32_t num_pages = dataSize / sectionLength;
    if (dataSize % sectionLength) // we need a page more because there is data left that does not fill a full page
    {
        num_pages++;
    }
    return num_pages;
}

static uint32_t getDFlashNumPages(size_t dataSize)
{
    return getNumPerSize(DFLASH_PAGE_LENGTH, dataSize, 1);
}

static uint32_t getDFlashNumSectors(size_t dataSize)
{
    return getNumPerSize(DFLASH_SECTOR_LENGTH, dataSize, 1);
}

static uint32_t getPFlashNumPages(size_t dataSize)
{
    return getNumPerSize(PFLASH_PAGE_LENGTH, dataSize, 1);
}

static uint32_t getPFlashNumSectors(size_t dataSize)
{
    return getNumPerSize(PFLASH_SECTOR_LENGTH, dataSize, 0);
}

static uint32_t getPFlashNumPhySectors()
{
    return getNumPerSize(PFLASH_SECTOR_LENGTH, PFLASH_PHY_SECTOR_LENGTH, 0);
}

static void readToVerifyPFlash(uint32 startAddr, uint32_t data[], size_t dataSize){

    FILE *f3 = fopen("terminal window 3", "rw");

    uint32_t dataIndex = 0;

    fprintf(f3, "Reading %zu uint32_t from 0x%X\n", (unsigned long)dataSize, startAddr);
    for (uint32_t address = startAddr; address < startAddr + dataSize * sizeof(uint32); address += sizeof(uint32))
    {
        uint32_t content = MEM(address);
        if ( content > 0)
        {
            if (content != data[dataIndex]){
                fprintf(f3, " -> Address 0x%X: Content is different Is: 0x%X, Should: 0x%X\n", address, content, data[dataIndex]);
            } else{
                //fprintf(f3, " -> Address 0x%X: Content is equal\n", address);
            }
        }
        else {
            //fprintf(f3, " -> Address 0x%X is 0\n", address);
        }
        dataIndex++;
    }

    fclose(f3);
}

static void erasePFlashSectors(IfxFlash_FlashType flashModule, uint32_t flashStartAddr, size_t dataSize){
    uint32_t num_loc_per_phy = getPFlashNumPhySectors(PFLASH_SECTOR_LENGTH);

    uint32_t num_bytes = 0;
    uint32_t num_sectors = 0;

    uint32_t num_sectors_to_erase = 0;
    uint32_t num_sectors_delta = 0;
    uint32_t erase_start_addr = 0;

    // Address belongs to core 0
    if(flashStartAddr >= pflash_eraser.core0_start_addr && flashStartAddr < pflash_eraser.core0_end_addr){
        num_bytes = (flashStartAddr + dataSize*sizeof(uint32_t)) - pflash_eraser.core0_start_addr;
        num_sectors = getPFlashNumSectors(num_bytes);

        num_sectors_delta = num_sectors - pflash_eraser.core0_erased_sections;
        while(num_sectors_delta > 0){
            erase_start_addr = pflash_eraser.core0_start_addr + (pflash_eraser.core0_erased_sections * PFLASH_SECTOR_LENGTH);
            if(num_sectors_delta > num_loc_per_phy){
                num_sectors_to_erase = num_loc_per_phy;
            }
            else {
                num_sectors_to_erase = num_sectors_delta;
            }
            g_functionsFromPSPR.erasePFlash(flashModule, erase_start_addr, num_sectors_to_erase);

            pflash_eraser.core0_erased_sections += num_sectors_to_erase;
            num_sectors_delta = num_sectors - pflash_eraser.core0_erased_sections;
        }
    }

    // Address belongs to core 1
    if(flashStartAddr >= pflash_eraser.core1_start_addr && flashStartAddr < pflash_eraser.core1_end_addr){
        num_bytes = (flashStartAddr + dataSize*sizeof(uint32_t)) - pflash_eraser.core1_start_addr;
        num_sectors = getPFlashNumSectors(num_bytes);

        num_sectors_delta = num_sectors - pflash_eraser.core1_erased_sections;
        while(num_sectors_delta > 0){
            erase_start_addr = pflash_eraser.core1_start_addr + (pflash_eraser.core1_erased_sections * PFLASH_SECTOR_LENGTH);
            if(num_sectors_delta > num_loc_per_phy){
                num_sectors_to_erase = num_loc_per_phy;
            }
            else {
                num_sectors_to_erase = num_sectors_delta;
            }
            g_functionsFromPSPR.erasePFlash(flashModule, erase_start_addr, num_sectors_to_erase);

            pflash_eraser.core1_erased_sections += num_sectors_to_erase;
            num_sectors_delta = num_sectors - pflash_eraser.core1_erased_sections;
        }
    }

    // Address belongs to core 2
    if(flashStartAddr >= pflash_eraser.core2_start_addr && flashStartAddr < pflash_eraser.core2_end_addr){
        num_bytes = (flashStartAddr + dataSize*sizeof(uint32_t)) - pflash_eraser.core2_start_addr;
        num_sectors = getPFlashNumSectors(num_bytes);

        num_sectors_delta = num_sectors - pflash_eraser.core2_erased_sections;
        while(num_sectors_delta > 0){
            erase_start_addr = pflash_eraser.core2_start_addr + (pflash_eraser.core2_erased_sections * PFLASH_SECTOR_LENGTH);
            if(num_sectors_delta > num_loc_per_phy){
                num_sectors_to_erase = num_loc_per_phy;
            }
            else {
                num_sectors_to_erase = num_sectors_delta;
            }
            g_functionsFromPSPR.erasePFlash(flashModule, erase_start_addr, num_sectors_to_erase);

            pflash_eraser.core2_erased_sections += num_sectors_to_erase;
            num_sectors_delta = num_sectors - pflash_eraser.core2_erased_sections;
        }
    }

    // Address belongs to asw key
    if(flashStartAddr >= pflash_eraser.asw_key_start_addr && flashStartAddr < pflash_eraser.asw_key_end_addr){
        num_bytes = (flashStartAddr + dataSize*sizeof(uint32_t)) - pflash_eraser.asw_key_start_addr;
        num_sectors = getPFlashNumSectors(num_bytes);

        num_sectors_delta = num_sectors - pflash_eraser.asw_key_erased_sections;
        while(num_sectors_delta > 0){
            erase_start_addr = pflash_eraser.asw_key_start_addr + (pflash_eraser.asw_key_erased_sections * PFLASH_SECTOR_LENGTH);
            if(num_sectors_delta > num_loc_per_phy){
                num_sectors_to_erase = num_loc_per_phy;
            }
            else {
                num_sectors_to_erase = num_sectors_delta;
            }
            g_functionsFromPSPR.erasePFlash(flashModule, erase_start_addr, num_sectors_to_erase);

            pflash_eraser.asw_key_erased_sections += num_sectors_to_erase;
            num_sectors_delta = num_sectors - pflash_eraser.asw_key_erased_sections;
        }
    }

    // Address belongs to cal data
    if(flashStartAddr >= pflash_eraser.cal_data_start_addr && flashStartAddr < pflash_eraser.cal_data_end_addr){
        num_bytes = (flashStartAddr + dataSize*sizeof(uint32_t)) - pflash_eraser.cal_data_start_addr;
        num_sectors = getPFlashNumSectors(num_bytes);

        num_sectors_delta = num_sectors - pflash_eraser.cal_data_erased_sections;
        while(num_sectors_delta > 0){
            erase_start_addr = pflash_eraser.cal_data_start_addr + (pflash_eraser.cal_data_erased_sections * PFLASH_SECTOR_LENGTH);
            if(num_sectors_delta > num_loc_per_phy){
                num_sectors_to_erase = num_loc_per_phy;
            }
            else {
                num_sectors_to_erase = num_sectors_delta;
            }
            g_functionsFromPSPR.erasePFlash(flashModule, erase_start_addr, num_sectors_to_erase);

            pflash_eraser.cal_data_erased_sections += num_sectors_to_erase;
            num_sectors_delta = num_sectors - pflash_eraser.cal_data_erased_sections;
        }
    }
}

/* This function flashes the Program Flash memory calling the routines from the PSPR */
static bool flashWriteProgram(IfxFlash_FlashType flashModule, uint32_t flashStartAddr, uint32_t data[], size_t dataSize)
{
    if(pflash_eraser.init == 0){
        flashDriverInit();
        if(pflash_eraser.init == 0) // Init was not successful
            return false;
    }

    uint32_t num_pages = getPFlashNumPages(dataSize);

    // Check for full pages and prepare last flash page, always use buffered last page for flashing to make sure full pages are written
    uint32_t last_page_addr = flashStartAddr + (num_pages-1) * PFLASH_PAGE_LENGTH;
    uint32_t *data_for_last_page = (uint32_t*) (((uint8*) data) + ((num_pages-1) * PFLASH_PAGE_LENGTH));
    createLastFlashPage(last_page_addr, data_for_last_page, dataSize%PFLASH_LAST_PAGE_SIZE == 0 ? PFLASH_LAST_PAGE_SIZE : dataSize%PFLASH_LAST_PAGE_SIZE);

    boolean interruptState = IfxCpu_disableInterrupts();

    copyFunctionsToPSPR(); // avoid overwriting functions while writing flash by copying them into PSPR

    erasePFlashSectors(flashModule, flashStartAddr, dataSize);
    g_functionsFromPSPR.writePFlash(flashModule, flashStartAddr, num_pages, data, dataSize);

    readToVerifyPFlash(flashStartAddr, data, dataSize);

    IfxCpu_restoreInterrupts(interruptState);
    return true;
}

/* This function flashes the Data Flash memory.
 * It is not needed to run this function from the PSPR, thus functions from the Program Flash memory can be called
 * inside.
 */
static bool flashWriteData(IfxFlash_FlashType flashModule, uint32_t flashStartAddr, uint32_t data[], size_t dataSize)
{
    uint32_t num_sectors = getDFlashNumSectors(dataSize);
    uint32_t num_pages = getDFlashNumPages(dataSize);

    uint32_t page;

    /* --------------- ERASE PROCESS --------------- */
    uint16 endInitSafetyPassword = IfxScuWdt_getSafetyWatchdogPassword(); /* Get the current password of the Safety WatchDog module */

    /* Erase the sector */
 
    IfxScuWdt_clearSafetyEndinit(endInitSafetyPassword);        /* Disable EndInit protection                       */
    IfxFlash_eraseMultipleSectors(flashStartAddr, num_sectors);
    IfxScuWdt_setSafetyEndinit(endInitSafetyPassword);          /* Enable EndInit protection                        */

    /* Wait until the sector is erased */
    IfxFlash_waitUnbusy(PMU_FLASH_MODULE, flashModule);

    /* --------------- WRITE PROCESS --------------- */
    for(page = 0; page < num_pages; page++)
    {
        uint32_t page_addr = flashStartAddr + (page * DFLASH_PAGE_LENGTH);
        uint32* data_for_page = (uint32*) (((uint8*) data) + (page * DFLASH_PAGE_LENGTH));

        IfxFlash_enterPageMode(page_addr);  // enter page mode to be able to write in page

        /* Wait until page mode is entered */
        IfxFlash_waitUnbusy(PMU_FLASH_MODULE, flashModule);


        IfxFlash_loadPage2X32(page_addr, data_for_page[0], data_for_page[1]); /* Load two words of 32 bits each */

        /* Write the loaded page */
        IfxScuWdt_clearSafetyEndinit(endInitSafetyPassword);    /* Disable EndInit protection                       */
        IfxFlash_writePage(page_addr);
        IfxScuWdt_setSafetyEndinit(endInitSafetyPassword);      /* Enable EndInit protection                        */

        /* Wait until the data is written in the Data Flash memory */
        IfxFlash_waitUnbusy(PMU_FLASH_MODULE, flashModule);
    }
    return true;
}

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/

/* This function inits the flash driver and makes sure that the CORE addresses are correctly setup for flashing into PFLASH */
void flashDriverInit(void){

    pflash_eraser.init = 0;
    pflash_eraser.core0_start_addr = flashGetDIDData(FBL_DID_BL_WRITE_START_ADD_CORE0);
    pflash_eraser.core0_end_addr = flashGetDIDData(FBL_DID_BL_WRITE_END_ADD_CORE0);
    pflash_eraser.core1_start_addr = flashGetDIDData(FBL_DID_BL_WRITE_START_ADD_CORE1);
    pflash_eraser.core1_end_addr = flashGetDIDData(FBL_DID_BL_WRITE_END_ADD_CORE1);
    pflash_eraser.core2_start_addr = flashGetDIDData(FBL_DID_BL_WRITE_START_ADD_CORE2);
    pflash_eraser.core2_end_addr = flashGetDIDData(FBL_DID_BL_WRITE_END_ADD_CORE2);
    pflash_eraser.asw_key_start_addr = flashGetDIDData(FBL_DID_BL_WRITE_START_ADD_ASW_KEY);
    pflash_eraser.asw_key_end_addr = flashGetDIDData(FBL_DID_BL_WRITE_END_ADD_ASW_KEY);
    pflash_eraser.cal_data_start_addr = flashGetDIDData(FBL_DID_BL_WRITE_START_ADD_CAL_DATA);
    pflash_eraser.cal_data_end_addr = flashGetDIDData(FBL_DID_BL_WRITE_END_ADD_CAL_DATA);
    flashResetErasedSectionsCtr();

    // First order conditions
    if( (pflash_eraser.core0_start_addr <= pflash_eraser.core0_end_addr) &&
        (pflash_eraser.core1_start_addr <= pflash_eraser.core1_end_addr) &&
        (pflash_eraser.core2_start_addr <= pflash_eraser.core2_end_addr) &&
        (pflash_eraser.asw_key_start_addr <= pflash_eraser.asw_key_end_addr) &&
        (pflash_eraser.cal_data_start_addr <= pflash_eraser.cal_data_end_addr)){

        // Second order conditions
        if((pflash_eraser.core0_end_addr - pflash_eraser.core0_start_addr + 1) % PFLASH_SECTOR_LENGTH != 0){
            if(pflash_eraser.core0_start_addr != pflash_eraser.core0_end_addr)
                return;
        }

        if((pflash_eraser.core1_end_addr - pflash_eraser.core1_start_addr + 1) % PFLASH_SECTOR_LENGTH != 0){
            if(pflash_eraser.core1_start_addr != pflash_eraser.core1_end_addr)
                return;
        }

        if((pflash_eraser.core2_end_addr - pflash_eraser.core2_start_addr + 1) % PFLASH_SECTOR_LENGTH != 0){
            if(pflash_eraser.core2_start_addr != pflash_eraser.core2_end_addr)
                return;
        }

        if((pflash_eraser.asw_key_end_addr - pflash_eraser.asw_key_start_addr + 1) % PFLASH_SECTOR_LENGTH != 0){
            if(pflash_eraser.asw_key_start_addr != pflash_eraser.asw_key_end_addr)
                return;
        }

        if((pflash_eraser.cal_data_end_addr - pflash_eraser.cal_data_start_addr + 1) % PFLASH_SECTOR_LENGTH != 0){
            if(pflash_eraser.cal_data_start_addr != pflash_eraser.cal_data_end_addr)
                return;
        }

        pflash_eraser.init = 1;
    }
}

/* This function resets the erased sections counters for the PFLASH */
void flashResetErasedSectionsCtr(void){
    pflash_eraser.core0_erased_sections = 0;
    pflash_eraser.core1_erased_sections = 0;
    pflash_eraser.core2_erased_sections = 0;
    pflash_eraser.asw_key_erased_sections = 0;
    pflash_eraser.cal_data_erased_sections = 0;
}

/* This function calls the correct writing function, either flashWriteProgramm or flashWriteData, depending on flashStartAddr,
 * so the programmer doesn't have to differentiate between writing pflash and dflash */
bool flashWrite(uint32_t flashStartAddr, uint32_t data[], size_t dataSize) {
    if (flashStartAddr >= DATA_FLASH_0_BASE_ADDR && flashStartAddr < DATA_FLASH_0_END_ADDR)
    {
        if (flashStartAddr + dataSize >= DATA_FLASH_0_END_ADDR) {
            return false;
        }
        return flashWriteData(DATA_FLASH_0, flashStartAddr, data, dataSize);
    }
    else if (flashStartAddr >= DATA_FLASH_1_BASE_ADDR && flashStartAddr < DATA_FLASH_1_END_ADDR)
    {
        if (flashStartAddr + dataSize >= DATA_FLASH_1_END_ADDR) {
            return false;
        }
        return flashWriteData(DATA_FLASH_1, flashStartAddr, data, dataSize);
    }
    else if (flashStartAddr >= PROGRAM_FLASH_0_BASE_ADDR && flashStartAddr < PROGRAM_FLASH_0_END_ADDR)
    {
        if (flashStartAddr + dataSize * sizeof(uint32) >= PROGRAM_FLASH_0_END_ADDR) 
        {
            return false;
        }
        return flashWriteProgram(PROGRAM_FLASH_0, flashStartAddr, data, dataSize);
    }
    else if (flashStartAddr >= PROGRAM_FLASH_1_BASE_ADDR && flashStartAddr < PROGRAM_FLASH_1_END_ADDR)
    {
        if (flashStartAddr + dataSize * sizeof(uint32) >= PROGRAM_FLASH_1_END_ADDR) 
        {
            return false;
        }
        return flashWriteProgram(PROGRAM_FLASH_1, flashStartAddr, data, dataSize);
    }
    return false;
} 

/* This function verifies that the data at the given address matches the data of the array data*/
bool flashVerify(uint32_t flashStartAddr, uint32_t data[], size_t dataSize)
{
    uint32_t dataIndex = 0;

    for (uint32_t address = flashStartAddr; address < flashStartAddr + dataSize * sizeof(uint32); address += sizeof(uint32))
    {
        if (MEM(address) != data[dataIndex])
        {
            return false;
        }
        dataIndex++;
    }
    return true;
}

/**
 * This function reads the given number of bytes into the dataReadFromFlash buffer. The caller need to make sure to free the buffer.
 * Internally the buffer is created by using calloc
 */
uint8_t *flashRead(uint32_t flashStartAddr, size_t dataBytesToRead){

    uint8_t *data = (uint8_t*)calloc((uint32_t)dataBytesToRead, sizeof(uint8_t));
    if(data != NULL){

        size_t uint32_bytes = dataBytesToRead / sizeof(uint32) + 1;
        uint32_t dataRead = 0;
        uint32_t byteCtr = 0;
        uint8_t *byteExtracted = 0;

        for(uint32_t address = flashStartAddr; (address < flashStartAddr + uint32_bytes * sizeof(uint32)) && byteCtr < dataBytesToRead; address += sizeof(uint32)){
            dataRead = MEM(address);
            byteExtracted = (uint8_t*)(&dataRead);

            for(int i = 0; i < sizeof(uint32) && byteCtr < dataBytesToRead; i++){
                data[byteCtr] = *byteExtracted;
                byteExtracted++;
                byteCtr++;
            }
        }
    }
    return data;
}

/**  
 * Calculates Checksum for the given part of memory used to verify flash after flashing 
 * Caution: CRC-Calculation is based on ASCII not Hex-number
 */

uint32_t flashCalculateChecksum(uint32_t flashStartAddr, uint32_t length) {

    char flashContent[3];
    flashContent[2] = '\0';
    uint32_t addr = flashStartAddr;
    uint32_t endAddr = flashStartAddr + length;

    crc_t crc = crc_init();
    uint32_t nextFourBytes = 0;
    while (addr < endAddr) {
        // For Debugging
        //if(addr >= 0xA04F8000)
        //    __nop();

        nextFourBytes = MEM(addr);

        for (int i = 0; i < 4; i++) {
            if (addr + i >= endAddr) {
                break;
            }
            uint8_t lower = (uint8_t)(nextFourBytes & 0x0000000F);
            lower += lower > 9 ? 0x37 : 0x30;
            nextFourBytes = nextFourBytes >> 4;
            uint8_t higher = (uint8_t)(nextFourBytes & 0x0000000F);
            higher += higher > 9 ? 0x37 : 0x30;
            nextFourBytes = nextFourBytes >> 4;

            flashContent[0] = higher;
            flashContent[1] = lower;

            crc = crc_update(crc, (const char *) flashContent, strlen(flashContent));
        }

        addr += 4;
    }

    crc = crc_finalize(crc);

    return (uint32_t) crc;
}
