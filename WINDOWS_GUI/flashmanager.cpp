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

void FlashManager::setUDS(UDS *uds){
    this->uds = uds;
}

void FlashManager::setFile(QString file){
    this->file = file;

    // TESTING
    if(file.endsWith(".test")){

        emit infoPrint("TESTMODE detected. Creating demo data");

        // Content for flashing
        QByteArray flashdate = getCurrentFlashDate();

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
    else{
        emit errorPrint("FlashManager: Flashing of files is not yet implemented");

        // TODO: set flashContent for single addresses
    }
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

size_t FlashManager::getOverallByteSize(){
    size_t numberOfBytes = 0;

    for(uint32_t add: flashContent.keys()){
        QByteArray bytes = flashContent[add];
        if(bytes != nullptr)
            numberOfBytes += bytes.size();
    }

    return numberOfBytes;
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

            case EXECUTE:
                executeFlashing();
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

            if(curr_state == EXECUTE){
                // Force Transfer Exit for any address
                uds->requestTransferExit(ecu_id, 0);
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

    curr_state = EXECUTE;
}

void FlashManager::executeFlashing(){

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

    UDS::RESP resp = UDS::RESP::RX_NO_RESPONSE;
    for(uint32_t add : flashContent.keys()){

        // TODO: Handle multiple calls of executeFlashing (error path) -> e.g. set content for add to zero and skip zero content.

        QByteArray bytes = flashContent[add];
        uint8_t *data = (uint8_t*) bytes.data();
        emit updateStatus(INFO, "Flashing "+QString::number(bytes.size())+" bytes to flash address "+QString("0x%8").arg(add, 8, 16, QLatin1Char( '0' )), 0);

        // ##############################################################################################################
        // Request Download
        emit infoPrint("Requesting Download for flash address "+QString("0x%8").arg(add, 8, 16, QLatin1Char( '0' )));
        resp = uds->requestDownload(ecu_id, add, bytes.size());

        if(resp != UDS::TX_RX_OK){
            emit errorPrint("ERROR: Requesting Download failed");
            return;
        }

        uint32_t bufferSize = uds->getECUTransferDataBufferSize();
        if(bufferSize == 0){
            emit errorPrint("ERROR: ECU Buffer size is 0.");
            return;
        }

        emit infoPrint("Requesting Download OK");

        mutex.lock();
        abort = _abort;
        mutex.unlock();

        if(abort)
            return;

        // ##############################################################################################################
        // Transfer Data

        // Info: content for single address need to be split according to buffer size. Each package need to be requested independently

        // Transfer Data
        uint32_t packages = bytes.size() % bufferSize > 0 ? bytes.size() / bufferSize + 1 : bytes.size() / bufferSize;
        QString info = "According to the buffer size of the ECU the data need to be splittet into "+QString::number(packages)+" packages";
        emit infoPrint(info);
        emit updateStatus(INFO, info, 0);

        uint32_t curr_flash_add = add;
        uint32_t curr_flash_byte_ptr = 0;
        uint32_t curr_flash_bytes = 0;

        for(int package = 0; package < packages; package++){
            curr_flash_add = add + package*bufferSize;
            curr_flash_byte_ptr = package*bufferSize;

            // Calc the bytes to be flashed
            if(curr_flash_add + bufferSize < add+flashContent[add].size())
                curr_flash_bytes = bufferSize;
            else
                curr_flash_bytes = add + flashContent[add].size() - curr_flash_add; // Last Packages

            emit infoPrint("Package "+QString::number(package+1)+": Transfer Data for flash address "+QString("0x%8").arg(curr_flash_add, 8, 16, QLatin1Char( '0' ))+ " ("+QString::number(curr_flash_bytes)+" bytes)");
            resp = uds->transferData(ecu_id, curr_flash_add, data+curr_flash_byte_ptr, curr_flash_bytes);

            if(resp != UDS::TX_RX_OK){
                emit errorPrint("ERROR: Transfer Data failed");
                return;
            }

            // Update the GUI progress bar
            flashedBytesCtr += curr_flash_bytes;
            emit updateStatus(UPDATE, "", (size_t)(((double)flashedBytesCtr)/getOverallByteSize()*100));

            mutex.lock();
            abort = _abort;
            mutex.unlock();

            if(abort)
                return;

        }

        resp = uds->requestTransferExit(ecu_id, add);
        if(resp != UDS::TX_RX_OK){
            emit errorPrint("ERROR: Transfer Exit failed");
            return;
        }
    }

    emit updateStatus(INFO, "Flash file fully transmitted.", 0);

    QThread::msleep(2000);
    curr_state = FINISH;
}

void FlashManager::finishFlashing(){

    emit infoPrint("###############################\nFlashManager: Finish Flashing Process\n###############################\n");

    // =========================================================================
    // Update Programming Date
    QByteArray flashdate = getCurrentFlashDate();
    uint8_t *data = (uint8_t*)flashdate.data();
    //uds->writeDataByIdentifier(ecu_id, FBL_DID_PROGRAMMING_DATE, data, flashdate.size());


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
