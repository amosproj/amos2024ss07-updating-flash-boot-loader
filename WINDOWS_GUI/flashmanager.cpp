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
    queuedGUIConsoleLog("###############################################\nTESTMODE detected. Creating demo data", 0);

    // ####################################################################
    // Core 0 ASW

    // Content for flashing
    QByteArray flashdateCore0 = getCurrentFlashDate();
    queuedGUIConsoleLog("Demodata Core 0: "+flashdateCore0.toHex());

    // Create some test bytes to flash to MCU
    QByteArray testBytesCore0;
    testBytesCore0.resize((size_t)TESTFILE_CORE0_BYTES);

    uint32_t testDataCtrCore0 = 0;
    for(int i = 0; i < testBytesCore0.size(); i++){
        if(testDataCtrCore0 % TESTFILE_PADDING_BYTES == 0)
            testDataCtrCore0 = 0;

        if(testDataCtrCore0 < flashdateCore0.size())
            testBytesCore0[i] = flashdateCore0[testDataCtrCore0];
        else
            testBytesCore0[i] = 0x00;

        testDataCtrCore0++;
    }
    flashContent[TESTFILE_CORE0_START_ADD] = testBytesCore0;

    // ####################################################################
    // Core 1 ASW

    // Content for flashing
    QByteArray flashdateCore1 = getCurrentFlashDate();
    queuedGUIConsoleLog("Demodata Core 1: "+flashdateCore1.toHex());

    // Create some test bytes to flash to MCU
    QByteArray testBytesCore1;
    testBytesCore1.resize((size_t)TESTFILE_CORE1_BYTES);

    uint32_t testDataCtrCore1 = 0;
    for(int i = 0; i < testBytesCore1.size(); i++){
        if(testDataCtrCore1 % TESTFILE_PADDING_BYTES == 0)
            testDataCtrCore1 = 0;

        if(testDataCtrCore1 < flashdateCore1.size())
            testBytesCore1[i] = flashdateCore1[testDataCtrCore1];
        else
            testBytesCore1[i] = 0x00;

        testDataCtrCore1++;
    }
    flashContent[TESTFILE_CORE1_START_ADD] = testBytesCore1;

    // ####################################################################
    // ASW Key Start

    // Content for flashing
    QByteArray flashdateASWKey = getCurrentFlashDate();
    queuedGUIConsoleLog("Demodata ASW Key: "+flashdateASWKey.toHex());

    // Create some test bytes to flash to MCU
    QByteArray testBytesASWKey;
    testBytesASWKey.resize((size_t)TESTFILE_ASW_KEY_BYTES);

    uint32_t testDataCtrASWKey = 0;
    for(int i = 0; i < testBytesASWKey.size(); i++){
        if(testDataCtrASWKey % TESTFILE_PADDING_BYTES == 0)
            testDataCtrASWKey = 0;

        if(testDataCtrASWKey < flashdateASWKey.size())
            testBytesASWKey[i] = flashdateASWKey[testDataCtrASWKey];
        else
            testBytesASWKey[i] = 0x00;

        testDataCtrASWKey++;
    }
    flashContent[TESTFILE_ASW_KEY_START_ADD] = testBytesASWKey;

    // ####################################################################
    // ASW Calibration Data

    // Content for flashing
    QByteArray flashdateCALData = getCurrentFlashDate();
    queuedGUIConsoleLog("Demodata Calibration Data: "+flashdateCALData.toHex());

    // Create some test bytes to flash to MCU
    QByteArray testBytesCalData;
    testBytesCalData.resize((size_t)TESTFILE_CAL_DATA_BYTES);

    uint32_t testDataCtrCalData = 0;
    for(int i = 0; i < testBytesCalData.size(); i++){
        if(testDataCtrCalData % TESTFILE_PADDING_BYTES == 0)
            testDataCtrCalData = 0;

        if(testDataCtrCalData < flashdateCALData.size())
            testBytesCalData[i] = flashdateCALData[testDataCtrCalData];
        else
            testBytesCalData[i] = 0x00;

        testDataCtrCalData++;
    }
    flashContent[TESTFILE_CAL_DATA_START_ADD] = testBytesCalData;
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

QMap<uint32_t, QByteArray> FlashManager::extractDataFromTestFile(QMap<uint32_t, QByteArray> compressedData) {
    QMap<uint32_t, QByteArray> result;

    for (auto [key, value] : compressedData.asKeyValueRange()) {
        QByteArray splitBytes;
        splitBytes.resize(2 * value.size());

        for (uint32_t i = 0; i < value.size(); i++) {
            int byte = value[i];
            uint32_t lower = byte & 0x0000000F;
            lower += lower > 9 ? 0x37 : 0x30;
            byte = byte >> 4;
            uint32_t higher = byte & 0x0000000F;
            higher += higher > 9 ?  0x37 : 0x30;
            splitBytes[2 * i] = (char) higher;
            splitBytes[2 * i + 1] = (char) lower;
        }

        result.insert(key, splitBytes);
    }



    return result;
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
    size_t new_value = (size_t)(bytes_counter/getOverallByteSize()*100);
    if(new_value != last_update_gui_progressbar){
        last_update_gui_progressbar = new_value;
        emit updateStatus(UPDATE, "", new_value);
    }
}

void FlashManager::queuedGUIConsoleLog(QString info, bool forced){
    bool update = false;
    if(forced || lastGUIUpdateConsoleLog.msecsTo(QDateTime::currentDateTime()) > TIME_DELTA_GUI_LOG){
        update = true;
    }

    if(!info.isEmpty())
        queueGUIConsoleLog.enqueue(info);

    if (update){
        QString updateString = "";
        while(!queueGUIConsoleLog.isEmpty()){
            updateString.append(queueGUIConsoleLog.dequeue() + "\n");
        }
        if(!updateString.isEmpty()){
            emit infoPrint(updateString);
            lastGUIUpdateConsoleLog = QDateTime::currentDateTime();
        }
    }
}

void FlashManager::queuedGUIFlashingLog(FlashManager::STATUS s, QString info, bool forced){

    if(s != INFO && s != ERR){
        qInfo() << "queuedGUIFlashingLog - Wrong usage.";
        return;
    }

    bool update = false;

    if(forced || lastGUIUpdateFlashingLog.msecsTo(QDateTime::currentDateTime()) > TIME_DELTA_GUI_FLASHING_LOG){
        update = true;
    }

    if(!info.isEmpty()){
        QPair<STATUS, QString> addValue;
        addValue.first = s;
        addValue.second = info;
        queueGUIFlashingLog.enqueue(addValue);
    }

    if(update){
        STATUS tempStatus = INFO;
        QString updateString = "";
        while(!queueGUIFlashingLog.isEmpty()){
            QPair<STATUS, QString> queueVal = queueGUIFlashingLog.dequeue();
            if(queueVal.first == tempStatus){
                updateString.prepend(queueVal.second + "\n");
            }
            else{
                // Detected change in status, send out to GUI flashing log
                emit updateStatus(tempStatus, updateString, 0);

                updateString = "";
                tempStatus = queueVal.first;
                updateString.prepend(queueVal.second);
            }
        }

        if(!updateString.isEmpty()){
            // send out to GUI flashing log
            emit updateStatus(tempStatus, updateString, 0);
            lastGUIUpdateFlashingLog = QDateTime::currentDateTime();
        }
    }
}

//============================================================================
// Private Method
//============================================================================

void FlashManager::doFlashing(){

    emit flashingThreadStarted();
    qInfo() << "FlashManager: Started flashing.\n";
    queuedGUIConsoleLog("###############################################\nFlashManager: Started flashing.\n###############################################\n");

    while(this->_working) {
        // Check if thread should be canceled
        mutex.lock();
        bool abort = _abort;
        mutex.unlock();

        if(abort)
            break;

        // Check the print queues
        queuedGUIConsoleLog("");
        queuedGUIFlashingLog(INFO, "");

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

        // Check the print queues
        queuedGUIConsoleLog("");
        queuedGUIFlashingLog(INFO, "");

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
            emit errorPrint("\nFlashManager: Change to next state not possible. Waiting "+QString::number((uint32_t)WAITTIME_AFTER_ATTEMPT) + " ms before starting next attempt\n\n");
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

        // Check the print queues
        queuedGUIConsoleLog("");
        queuedGUIFlashingLog(INFO, "");
    }

    // Set _working to false, meaning the process can't be aborted anymore.
    mutex.lock();
    _working = false;
    mutex.unlock();

    // Reset progress bar
    queuedGUIFlashingLog(INFO, "", 1);
    emit updateStatus(FlashManager::UPDATE, "", 0);

    qInfo() << "FlashManager: Stopped flashing.\n";
    queuedGUIConsoleLog("###############################################\nFlashManager: Stopped flashing.\n###############################################\n");
    emit flashingThreadFinished();
}

void FlashManager::prepareFlashing(){

    queuedGUIConsoleLog("###############################\nFlashManager: Preparing Flashing Process\n###############################\n");

    // =========================================================================
    // Prepare GUI

    lastGUIUpdateConsoleLog = QDateTime::currentDateTime();
    lastGUIUpdateFlashingLog = QDateTime::currentDateTime();

    emit updateStatus(RESET, "", 0);
    queuedGUIFlashingLog(INFO, "Preparing ECU for flashing");
    last_update_gui_progressbar = 0;

    // =========================================================================
    // Prepare ECU
    UDS::RESP resp = UDS::RESP::RX_NO_RESPONSE;

    queuedGUIConsoleLog("Change Session to Programming Session for selected ECU");
    resp = uds->diagnosticSessionControl(ecu_id, FBL_DIAG_SESSION_PROGRAMMING);

    if(resp != UDS::TX_RX_OK){
        // Check on response more detailed
        if(uds->getECUNegativeResponse() > 0){
            // Negative Response received, ECU is responding
            // Strategy: Try again
            return;
        }

        // No Response from ECU
        curr_state = ERR_STATE;
        emit errorPrint("No Response from selected ECU - Aborting.");
        return;
    }

    //queuedGUIConsoleLog("TODO: Add Security Access once activated");
    //resp = uds->securityAccessRequestSEED(ecu_id);

    // Reset the counter for flashed bytes
    flashedBytesCtr = 0;
    flashedBytes.clear();
    fillOverallByteSize();

    curr_state = START_FLASHING;
}

void FlashManager::startFlashing(){

    queuedGUIConsoleLog("###############################\nFlashManager: Executing Flashing\n###############################\n");

    if(flashContent.isEmpty()){
        emit errorPrint("FlashManager: Provided flash file has no content");
        queuedGUIFlashingLog(ERR, "Provided flash file has no content");
        curr_state = IDLE;
        return;
    }

    mutex.lock();
    bool abort = _abort;
    mutex.unlock();

    if(abort)
        return;

    queuedGUIFlashingLog(INFO, "Starting with flashing");

    // Setup the variables
    flashCurrentAdd = flashContent.firstKey();
    flashCurrentPackageCtr = 0;
    curr_state = REQ_DOWNLOAD;
}

void FlashManager::requestDownload(){

    mutex.lock();
    bool abort = _abort;
    mutex.unlock();

    if(abort)
        return;

    if(flashCurrentAdd == 0){
        emit errorPrint("FlashManager: Flash Address is not setup for Request Download");
        queuedGUIFlashingLog(ERR, "Flash Address is not setup for Request Download");
        curr_state = IDLE;
        return;
    }

    // ##############################################################################################################
    // Request Download
    UDS::RESP resp = UDS::RESP::RX_NO_RESPONSE;

    QByteArray bytes = flashContent[flashCurrentAdd];
    queuedGUIFlashingLog(INFO, "Flashing "+QString::number(bytes.size())+" bytes to flash address "+QString("0x%8").arg(flashCurrentAdd, 8, 16, QLatin1Char( '0' )));

    //queuedGUIConsoleLog("Requesting Download for flash address "+QString("0x%8").arg(flashCurrentAdd, 8, 16, QLatin1Char( '0' )));
    resp = uds->requestDownload(ecu_id, flashCurrentAdd, bytes.size());

    if(resp != UDS::TX_RX_OK){

        // Check on response more detailed
        if(uds->getECUNegativeResponse() > 0){
            // Negative Response received, ECU is responding
            // Strategy: Try again
            return;
        }

        // No Response from ECU
        curr_state = ERR_STATE;
        emit errorPrint("No Response from selected ECU - Aborting.");
        return;

    }

    flashCurrentBufferSize = uds->getECUTransferDataBufferSize();
    if(flashCurrentBufferSize == 0){
        emit errorPrint("ERROR: ECU Buffer size is 0.");
        return;
    }

    // Calculate the packages
    flashCurrentPackages = bytes.size() % flashCurrentBufferSize > 0 ? bytes.size() / flashCurrentBufferSize + 1 : bytes.size() / flashCurrentBufferSize;
    QString info = "Request Download OK for flash address "+QString("0x%8").arg(flashCurrentAdd, 8, 16, QLatin1Char( '0' ))+" (Buffer size="+QString::number(flashCurrentBufferSize)+", Packages="+QString::number(flashCurrentPackages)+")";
    queuedGUIConsoleLog(info);
    qInfo() << info;

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

        //queuedGUIConsoleLog("Package "+QString::number(package+1)+"/"+QString::number(flashCurrentPackages)+": Transfer Data for flash address "+QString("0x%8").arg(curr_flash_add, 8, 16, QLatin1Char( '0' ))+ " ("+QString::number(curr_flash_bytes)+" bytes)");
        resp = uds->transferData(ecu_id, curr_flash_add, data+curr_flash_byte_ptr, curr_flash_bytes);

        if(resp != UDS::TX_RX_OK){
            // Check on response more detailed
            if(uds->getECUNegativeResponse() > 0){
                // Negative Response received, ECU is responding
                // Strategy: Try again
                return;
            }

            // No Response from ECU
            curr_state = ERR_STATE;
            emit errorPrint("No Response from selected ECU - Aborting.");
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

        // Check the print queues
        queuedGUIConsoleLog("");
        queuedGUIFlashingLog(INFO, "");
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

    queuedGUIFlashingLog(INFO, "Flash file fully transmitted.");

    QThread::msleep(2000);
    curr_state = VALIDATE;
}

void FlashManager::validateFlashing(){

    for (auto [key, value] : fileChecksums.asKeyValueRange()) {
        
        UDS::RESP resp = UDS::RESP::RX_NO_RESPONSE;

        resp = uds->requestUpload(ecu_id, key, addressToLength.value(key));

        if (resp != UDS::TX_RX_OK) {
            emit errorPrint("FlashManager: ERROR - Requesting upload failed");

        }

        uint32_t ecuChecksum = uds->getECUChecksum();
        qInfo() << "Block with address 0x" + QString::number(key, 16) + " and length: "  + QString::number(addressToLength.value(key));

        if (ecuChecksum != value) {
            queuedGUIConsoleLog("FlashManager: ERROR in Block with address 0x" + QString::number(key, 16) + " and length: "  + QString::number(addressToLength.value(key)) + " - Calculated checksums didn't match.");
            qInfo() << "FlashManager: ERROR in Block with address 0x" + QString::number(key, 16) + " and length: "  + QString::number(addressToLength.value(key)) + " - Calculated checksums didn't match.";
            queuedGUIConsoleLog("Should be: 0x" + QString::number(value, 16) + ", but was: 0x" + QString::number(ecuChecksum, 16) + "\n");
            qInfo() << "Should be: 0x" + QString::number(value, 16) + ", but was: 0x" + QString::number(ecuChecksum, 16) + "\n";
            curr_state = ERR_STATE;
        }
        else {
             qInfo() << "IO - Should be: 0x" + QString::number(value, 16) + ", was: 0x" + QString::number(ecuChecksum, 16) + "\n";
        }
    }

    emit infoPrint("FlashManager: Flashing successful");

    curr_state = FINISH;
}

void FlashManager::finishFlashing(){

    queuedGUIConsoleLog("###############################\nFlashManager: Finish Flashing Process\n###############################\n");

    // =========================================================================
    // Update Programming Date
    QByteArray flashdate = getCurrentFlashDate();
    uint8_t *data = (uint8_t*)flashdate.data();
    uds->writeDataByIdentifier(ecu_id, FBL_DID_PROGRAMMING_DATE, data, flashdate.size());

    // =========================================================================
    // Update GUI
    queuedGUIFlashingLog(INFO, "Flashing finished!");

    curr_state = IDLE;
}

//============================================================================
// Slots
//============================================================================

void FlashManager::runThread(){
    this->doFlashing();
}


void FlashManager::forwardToConsole(const QString &text){
    queuedGUIConsoleLog(text);
}
