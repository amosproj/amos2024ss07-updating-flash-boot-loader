// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : flashmanager.cpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Flashmanger to flash ECUs
//============================================================================

#include "flashmanager.h"
#include <QDate>
#include <QTime>
#include <QTimer>
#include <QThread>
#include <QString>

#include "UDS_Spec/uds_comm_spec.h"

//============================================================================
// Constructor
//============================================================================

FlashManager::FlashManager(QObject *parent): QObject(parent){

    // Register STATUS Enum to make it usable for Signals&Slots
    qRegisterMetaType<FlashManager::STATUS>("FlashManager::STATUS");

    this->ecu_id = 0;
    this->uds = 0;
    this->file = "";

    // Flashing Thread is stopped by default
    this->_working =false;
    this->_abort = false;
}

FlashManager::~FlashManager(){
    this->stopFlashing();
}


//============================================================================
// Public Method
//============================================================================

void FlashManager::setTestFile(){
    // TESTING
    emit infoPrint("TESTMODE detected. Creating demo data");

    // Content for flashing
    QByteArray flashdate = getCurrentFlashDate();
    emit infoPrint("Demodata: "+flashdate.toHex());

    // Create some test bytes to flash to MCU
    QByteArray testBytes;
    testBytes.resize((size_t)TESTFILE_BYTES);

    uint32_t testDataCtr = 0;
    for(int i = 0; i < testBytes.size(); i++){
        if(testDataCtr % TESTFILE_PADDING_BYTES == 0)
            testDataCtr = 0;

        if(testDataCtr < flashdate.size())
            testBytes[i] = flashdate[testDataCtr];
        else
            testBytes[i] = 0x00;

        testDataCtr++;
    }
    flashContent[TESTFILE_START_ADD] = testBytes;
}

void FlashManager::setFlashFile(QMap<uint32_t, QByteArray> data){
    flashContent.clear();
    flashContent = data;
}

void FlashManager::setFileChecksums(QMap<uint32_t, uint32_t> checksums) {
    fileChecksums.clear();
    fileChecksums = checksums;
}

void FlashManager::setLengths(QMap<uint32_t, uint32_t> lengths) {
    addressToLength.clear();
    addressToLength = lengths;
}

QMap<uint32_t, QByteArray> FlashManager::getFlashContent(void) {
    return flashContent;
}

//============================================================================
// Private Helper Method
//============================================================================

QByteArray FlashManager::getCurrentFlashDate(){

    QDate date = QDate().currentDate();
    QTime time = QTime().currentTime();
    QString date_str = date.toString("ddMMyy");
    QString time_str = time.toString("HHmmss");
    QString combi = date_str + time_str;
    QByteArray bcd = QByteArray::fromHex(QByteArray(combi.toLocal8Bit()));

    return bcd;
}

void FlashManager::fillOverallByteSize(){

    for(uint32_t add: flashContent.keys()){
        flashContentSize[add] = flashContent[add].size();
    }
}

size_t FlashManager::getOverallByteSize(){
    size_t numberOfBytes = 0;

    for(uint32_t add: flashContentSize.keys()){
        numberOfBytes += flashContentSize[add];
    }

    return numberOfBytes;
}

void FlashManager::updateGUIProgressBar(){

    double bytes_counter = 0;
    for(uint32_t add: flashedBytes.keys()){
        bytes_counter += flashedBytes[add];
    }
    emit updateStatus(UPDATE, "", (size_t)(bytes_counter/getOverallByteSize()*100));
}

// TODO: REMOVE AFTER DEBUGGING
void FlashManager::own_sleep(uint32_t millis){
    QDateTime start = QDateTime::currentDateTime();
    while(start.msecsTo(QDateTime::currentDateTime()) < millis){};
}

//============================================================================
// Private Method
//============================================================================

void FlashManager::doFlashing(){

    qInfo() << "FlashManager: Started flashing.\n";
    emit infoPrint("###############################################\nFlashManager: Started flashing.\n###############################################\n");
    emit flashingThreadStarted();

    while(this->_working) {
        // Check if thread should be canceled
        mutex.lock();
        bool abort = _abort;
        mutex.unlock();

        if(abort)
            break;

        // Set the previous state to the last known current state
        prev_state = curr_state;

        // Count up the attempts of current state
        state_attempt_ctr++;

        // Handling for State Machine
        switch(curr_state){
            case PREPARE:
                prepareFlashing();
                break;

            case START_FLASHING:
                startFlashing();
                break;

            case REQ_DOWNLOAD:
                requestDownload();
                break;

            case TRANSFER_DATA:
                transferData();
                break;

            case VALIDATE:
                validateFlashing();
                break; 

            case FINISH:
                finishFlashing();
                break;

            case IDLE:
                stopFlashing();
                break;

            case ERR_STATE:
                qInfo() << "FlashManager: Aborting flashing.";
                emit errorPrint("###############################\nFlashManager: Aborting flashing.\n###############################\n");
                stopFlashing();
                break;

            default:
                QThread::msleep(200);
                break;
        }

        // Check if execution of current state changed it to the next one
        if(curr_state != prev_state)
            state_attempt_ctr = 0;

        if(state_attempt_ctr >= MAX_TRIES_PER_STATE){
            qInfo() << "FlashManager: ERROR - Reached max attempts for current state. Aborting\n";
            emit errorPrint("FlashManager: ERROR - Reached max attempts for current state. Aborting\n");
            curr_state = ERR_STATE;
            continue;
        }

        if(state_attempt_ctr > 0 && curr_state != IDLE){
            qInfo() << "\nFlashManager: Change to next state not possible. Waiting "+QString::number((uint32_t)WAITTIME_AFTER_ATTEMPT) + " ms before starting next attempt\n\n";
            emit infoPrint("\nFlashManager: Change to next state not possible. Waiting "+QString::number((uint32_t)WAITTIME_AFTER_ATTEMPT) + " ms before starting next attempt\n\n");
            QThread::msleep((unsigned long)WAITTIME_AFTER_ATTEMPT);

            if(curr_state == TRANSFER_DATA){
                // Force Transfer Exit for the current address
                uds->requestTransferExit(ecu_id, flashCurrentAdd);

                // Reset the package counter => Need to restart the flashing for first address
                flashCurrentPackageCtr = 0; // No Reset: Partial Flashing allowed

                // Change to Request Download again
                curr_state = REQ_DOWNLOAD;
            }
        }
    }

    // Set _working to false, meaning the process can't be aborted anymore.
    mutex.lock();
    _working = false;
    mutex.unlock();

    qInfo() << "FlashManager: Stopped flashing.\n";
    emit infoPrint("###############################################\nFlashManager: Stopped flashing.\n###############################################\n");
    emit flashingThreadFinished();

    // Reset progress bar
    emit updateStatus(FlashManager::UPDATE, "", 0);
}

void FlashManager::prepareFlashing(){

    emit infoPrint("###############################\nFlashManager: Preparing Flashing Process\n###############################\n");

    // =========================================================================
    // Prepare GUI

    emit updateStatus(RESET, "", 0);
    emit updateStatus(INFO, "Preparing ECU for flashing", 0);

    // =========================================================================
    // Prepare ECU
    UDS::RESP resp = UDS::RESP::RX_NO_RESPONSE;

    emit infoPrint("Change Session to Programming Session for selected ECU");
    resp = uds->diagnosticSessionControl(ecu_id, FBL_DIAG_SESSION_PROGRAMMING);

    if(resp != UDS::TX_RX_OK)
        return;

    emit infoPrint("TODO: Add Security Access once activated");
    //resp = uds->securityAccessRequestSEED(ecu_id);

    // Reset the counter for flashed bytes
    flashedBytesCtr = 0;
    flashedBytes.clear();
    fillOverallByteSize();

    curr_state = START_FLASHING;
}

void FlashManager::startFlashing(){

    emit infoPrint("###############################\nFlashManager: Executing Flashing\n###############################\n");

    // emit updateStatus(UPDATE, "Flashing started for " + ui->label_selected_ECU->text(), 0);

    if(flashContent.isEmpty()){
        emit errorPrint("FlashManager: Provided flash file has no content");
        emit updateStatus(ERR, "Provided flash file has no content", 0);
        curr_state = IDLE;
        return;
    }

    mutex.lock();
    bool abort = _abort;
    mutex.unlock();

    if(abort)
        return;

    emit updateStatus(INFO, "Starting with flashing", 0);

    // Setup the variables
    flashCurrentAdd = flashContent.firstKey();
    flashCurrentPackageCtr = 0;
    curr_state = REQ_DOWNLOAD;
}

void FlashManager::requestDownload(){

    //own_sleep(200); // TODO: REMOVE AFTER DEBUGGING

    mutex.lock();
    bool abort = _abort;
    mutex.unlock();

    if(abort)
        return;


    if(flashCurrentAdd == 0){
        emit errorPrint("FlashManager: Flash Address is not setup for Request Download");
        emit updateStatus(ERR, "Flash Address is not setup for Request Download", 0);
        curr_state = IDLE;
        return;
    }

    if(flashCurrentPackageCtr > 0 && flashContent.firstKey() == flashCurrentAdd){
        emit updateStatus(INFO, "Request Download for Flash Address "+QString("0x%8").arg(flashCurrentAdd, 8, 16, QLatin1Char( '0' ))+" already done.", 0);
        curr_state = TRANSFER_DATA;
        return;
    }

    // ##############################################################################################################
    // Request Download
    UDS::RESP resp = UDS::RESP::RX_NO_RESPONSE;

    QByteArray bytes = flashContent[flashCurrentAdd];
    emit updateStatus(INFO, "Flashing "+QString::number(bytes.size())+" bytes to flash address "+QString("0x%8").arg(flashCurrentAdd, 8, 16, QLatin1Char( '0' )), 0);

    emit infoPrint("Requesting Download for flash address "+QString("0x%8").arg(flashCurrentAdd, 8, 16, QLatin1Char( '0' )));
    resp = uds->requestDownload(ecu_id, flashCurrentAdd, bytes.size());

    if(resp != UDS::TX_RX_OK){
        emit errorPrint("ERROR: Requesting Download failed");
        return;
    }

    flashCurrentBufferSize = uds->getECUTransferDataBufferSize();
    if(flashCurrentBufferSize == 0){
        emit errorPrint("ERROR: ECU Buffer size is 0.");
        return;
    }

    // Calculate the packages
    flashCurrentPackages = bytes.size() % flashCurrentBufferSize > 0 ? bytes.size() / flashCurrentBufferSize + 1 : bytes.size() / flashCurrentBufferSize;
    QString info = "Requesting Download OK. According to the buffer size of the ECU the data need to be split into "+QString::number(flashCurrentPackages)+" packages";
    emit infoPrint(info);
    emit updateStatus(INFO, info, 0);


    // Prepare the flashed bytes map
    flashedBytes[flashCurrentAdd] = 0;

    mutex.lock();
    abort = _abort;
    mutex.unlock();

    if(abort)
        return;

    curr_state = TRANSFER_DATA;
}

void FlashManager::transferData(){

    mutex.lock();
    bool abort = _abort;
    mutex.unlock();

    if(abort)
        return;

    uint32_t curr_flash_add = flashCurrentAdd;
    uint32_t curr_flash_byte_ptr = 0;
    uint32_t curr_flash_bytes = 0;

    QByteArray bytes = flashContent[flashCurrentAdd];
    uint8_t *data = (uint8_t*) bytes.data();

    UDS::RESP resp = UDS::RESP::RX_NO_RESPONSE;
    for(int package = flashCurrentPackageCtr; package < flashCurrentPackages; package++){
        curr_flash_add = flashCurrentAdd + package*flashCurrentBufferSize;
        curr_flash_byte_ptr = package*flashCurrentBufferSize;

        // Calc the bytes to be flashed
        if(curr_flash_add + flashCurrentBufferSize < flashCurrentAdd+flashContent[flashCurrentAdd].size())
            curr_flash_bytes = flashCurrentBufferSize;
        else
            curr_flash_bytes = flashCurrentAdd + flashContent[flashCurrentAdd].size() - curr_flash_add; // Last Packages

        emit infoPrint("Package "+QString::number(package+1)+"/"+QString::number(flashCurrentPackages)+": Transfer Data for flash address "+QString("0x%8").arg(curr_flash_add, 8, 16, QLatin1Char( '0' ))+ " ("+QString::number(curr_flash_bytes)+" bytes)");
        resp = uds->transferData(ecu_id, curr_flash_add, data+curr_flash_byte_ptr, curr_flash_bytes);

        //own_sleep(200); // TODO: REMOVE AFTER DEBUGGING

        if(resp != UDS::TX_RX_OK){
            emit errorPrint("ERROR: Transfer Data failed");
            return;
        }

        // Transfer Data successfully, update package ctr
        flashCurrentPackageCtr = package;

        // Update the GUI progress bar
        //TODO: Fix progress bar if flashing starts again for same address - Currently only adding
        flashedBytes[flashCurrentAdd] += curr_flash_bytes;
        updateGUIProgressBar();

        mutex.lock();
        abort = _abort;
        mutex.unlock();

        if(abort)
            return;

    }

    resp = uds->requestTransferExit(ecu_id, flashCurrentAdd);
    if(resp != UDS::TX_RX_OK){
        emit errorPrint("ERROR: Transfer Exit failed");
        return;
    }

    // Update to the next flash address
    size_t itemsRemoved = flashContent.remove(flashCurrentAdd);
    if(!itemsRemoved){
        emit errorPrint("FlashManager: ERROR - Could not remove the flash address from Map\n");
    }

    if(flashContent.keys().count() > 0){
        flashCurrentPackageCtr = 0;
        flashCurrentAdd = flashContent.firstKey();
        curr_state = REQ_DOWNLOAD;
        return;
    }

    emit updateStatus(INFO, "Flash file fully transmitted.", 0);

    QThread::msleep(2000);
    curr_state = VALIDATE;
}

void FlashManager::validateFlashing(){

    for (auto [key, value] : fileChecksums.asKeyValueRange()) {
        
        UDS::RESP resp = UDS::RESP::RX_NO_RESPONSE;

        resp = uds->requestUpload(ecu_id, key, addressToLength.value(key));

        if (resp != UDS::TX_RX_OK) {
            emit errorPrint("FlashManager: ERROR - Requesting upload failed");
            break;
        }

        uint32_t ecuChecksum = uds->getECUChecksum();
        if (ecuChecksum == 0) {
            emit errorPrint("FlashManager: ERROR - Checksum transmission failed");
            break;
        }

        if (ecuChecksum != value) {
            emit errorPrint("FlashManager: ERROR in Block with address 0x" + QString::number(key, 16) + " and length: "  + QString::number(addressToLength.value(key)) + " - Calculated checksums didn't match.");
            emit errorPrint("Should be: 0x" + QString::number(value, 16) + ", but was: 0x" + QString::number(ecuChecksum, 16) + "\n");
            curr_state = ERR_STATE;
        }
    }

    emit infoPrint("FlashManager: Flashing successful");

    curr_state = FINISH;
}

void FlashManager::finishFlashing(){

    emit infoPrint("###############################\nFlashManager: Finish Flashing Process\n###############################\n");

    // =========================================================================
    // Update Programming Date
    QByteArray flashdate = getCurrentFlashDate();
    uint8_t *data = (uint8_t*)flashdate.data();
    uds->writeDataByIdentifier(ecu_id, FBL_DID_PROGRAMMING_DATE, data, flashdate.size());


    // =========================================================================
    // Update GUI
    emit updateStatus(INFO, "Flashing finished!", 0);

    curr_state = IDLE;
}

//============================================================================
// Slots
//============================================================================

void FlashManager::runThread(){
    this->doFlashing();
}


void FlashManager::forwardToConsole(const QString &text){
    emit infoPrint(text);
}
