// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : flashmanager.h
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Flashmanger to flash ECUs
//============================================================================

#ifndef FLASHMANAGER_H_
#define FLASHMANAGER_H_

#define VERBOSE_FLASHMANAGER        0           // switch for verbose console information
#define MAX_TRIES_PER_STATE         5           // Max Attempts per state
#define MAX_TRIES_PER_FLASH_ADD     10          // Max Attemps per flash address
#define WAITTIME_AFTER_ATTEMPT      500         // Waittime in ms
#define TIME_DELTA_GUI_LOG          500         // Delta in ms between GUI Updates for Console and Flashing log
#define TIME_DELTA_GUI_FLASHING_LOG 1000        // Delta in ms between GUI Updates for Console and Flashing log

#define TESTFILE_PADDING_BYTES      7           // Padding between test data
#define TESTFILE_CORE0_START_ADD    0xA0090000  // Start Address for flashing Core 0
#define TESTFILE_CORE0_BYTES        0x0016FFFF  // ~1.5 MB
#define TESTFILE_CORE1_START_ADD    0xA0304000  // Start Address for flashing Core 1
#define TESTFILE_CORE1_BYTES        0x001F3FFF  // ~2 MB
#define TESTFILE_ASW_KEY_START_ADD  0xA04F8000  // Start Address for flashing ASW Key
#define TESTFILE_ASW_KEY_BYTES      0x00003FFF  // ~16 KB
#define TESTFILE_CAL_DATA_START_ADD 0xA04FC000  // Start Address for flashing Calibration data
#define TESTFILE_CAL_DATA_BYTES     0x00003FFF  // ~16 KB

#define GOOD                        1
#define BAD                         0

#include <QObject>
#include <QMutex>
#include <QDebug>
#include <QQueue>
#include <QDateTime>
#include <QPair>

#include <stdint.h>

#include "UDS_Layer/UDS.hpp"
#include "Communication_Layer/Communication.hpp"

class FlashManager : public QObject {
    Q_OBJECT

public:
    enum STATUS {UPDATE, INFO, ERR, RESET};

private:
    enum STATE_MACHINE {PREPARE, START_FLASHING, REQ_DOWNLOAD, TRANSFER_DATA, VALIDATE, FINISH, IDLE, ERR_STATE};
    STATE_MACHINE curr_state, prev_state;                       // States
    uint8_t state_attempt_ctr;                                  // State attempt counter
    //uint8_t flash_add_attempt_ctr;                              // Flash address attempt counter
    uint32_t ecu_id;                                            // ECU ID to flash
    UDS *uds;                                                   // Reference to UDS Layer
    Communication *comm;                                        // Reference to Comm Layer
    QString file;                                               // Reference to file for flashing
    QByteArray updateVersion;                                   // ByteArray with Update Version content, to be written after flashing is finished
    QMap<uint32_t, QByteArray> flashContent;                    // Map with Address -> continous byte array
    QMap<uint32_t, uint32_t> flashContentSize;                  // Map with total size of content for every address
    QMap<uint32_t, uint32_t> flashedBytes;                      // Map with sum of flashed bytes for every address
    QMap<uint32_t, uint32_t> checksums;                         // Map of the checksums for every address

    size_t flashedBytesCtr;                                     // Counter for flashed bytes
    uint32_t flashCurrentAdd;                                   // Stores the current address to be flashed
    uint32_t flashCurrentPackages;                              // Stores the current number of packes for flashCurrentAdd;
    uint32_t flashCurrentBufferSize;                            // Stores the current buffer size per
    uint32_t flashCurrentPackageCtr;                            // Stores the current counter of the package

    size_t last_update_gui_progressbar;                         // Stores the last percent of the GUI progressbar

    QDateTime lastGUIUpdateConsoleLog;                          // Stores the last timestamp of Update of GUI Console
    QQueue<QString> queueGUIConsoleLog;                         // Stores Messages for the GUI Update
    QDateTime lastGUIUpdateFlashingLog;                         // Stores the last timestamp of Update of GUI Flashing Log
    QQueue<QPair<STATUS, QString>> queueGUIFlashingLog;                        // Stores Messages for the GUI Update

    bool _abort;                                                // Thread Handling
    bool _working;                                              // Thread Handling
    QMutex mutex;                                               // Protects _abort and _working

public:
    explicit FlashManager(QObject *parent = 0);
    virtual ~FlashManager();

    void setECUID(uint32_t ecu_id);
    void setTestFile();
    void setFlashFile(QMap<uint32_t, QByteArray> data);
    void setUpdateVersion(QByteArray version);
    QMap<uint32_t, QByteArray> getFlashContent(void);

    void startFlashing(uint32_t ecu_id, uint32_t gui_id, Communication* comm){

        if(ecu_id <= 0){
            emit errorPrint("FlashManager: Could not start flashing. Wrong ECU ID given");
            this->stopFlashing();
            return;
        }

        this->ecu_id = ecu_id;
        this->comm = comm;
        this->uds = new UDS(gui_id);

        // Disconnect everything from comm
        disconnect(comm, SIGNAL(rxDataReceived(uint, QByteArray)), 0, 0); // disconnect everything connect to rxDataReived
        disconnect(comm, SIGNAL(toConsole(QString)), 0, 0); // disconnect everything connect to toConsole

        // GUI Console Print
        connect(this->comm, SIGNAL(toConsole(QString)), this, SLOT(forwardToConsole(QString)), Qt::DirectConnection);

        // Comm RX Signal to UDS RX Slot
        connect(this->comm, SIGNAL(rxDataReceived(uint, QByteArray)), this->uds, SLOT(rxDataReceiverSlot(uint, QByteArray)), Qt::DirectConnection);

        // UDS TX Signals to Comm TX Slots
        connect(this->uds, SIGNAL(setID(uint32_t)),    this->comm, SLOT(setIDSlot(uint32_t)), Qt::DirectConnection);
        connect(this->uds, SIGNAL(txData(QByteArray)), this->comm, SLOT(txDataSlot(QByteArray)), Qt::DirectConnection);

        state_attempt_ctr = 0;
        curr_state = PREPARE;

        mutex.lock();
        _working = true;
        _abort = false;
        mutex.unlock();

        qInfo() << "FlashManager: Starting Flashing Thread";
        emit flashingStartThreadRequested();
    }

    void stopFlashing(){
        mutex.lock();
        if (_working) {
            _abort = true;
        }
        mutex.unlock();

        // Finally print all the left over queued logs
        queuedGUIConsoleLog("", 1);
        queuedGUIFlashingLog(INFO, "", 1);


        // Comm RX Signal to UDS RX Slot
        if(comm != nullptr){
            disconnect(comm, SIGNAL(rxDataReceived(uint, QByteArray)), 0, 0); // disconnect everything connect to rxDataReived
            disconnect(comm, SIGNAL(toConsole(QString)), 0, 0);
        }

        // UDS TX Signals to Comm TX Slots
        if(uds != nullptr){
            disconnect(uds, SIGNAL(setID(uint32_t)), 0, 0);
            disconnect(uds, SIGNAL(txData(QByteArray)), 0, 0);
        }

        qInfo() << "FlashManager: Stopping Flashing Thread";
        emit flashingStopThreadRequested();
    }

private:
    QByteArray getCurrentFlashDate();
    void fillOverallByteSize();
    size_t getOverallByteSize();
    void updateGUIProgressBar();
    void queuedGUIConsoleLog(QString info, bool forced=0);
    void queuedGUIFlashingLog(FlashManager::STATUS s, QString info, bool forced=0);
    QMap<uint32_t, uint32_t> calculateFileChecksums(QMap<uint32_t, QByteArray> data);
    QMap<uint32_t, QByteArray> uncompressData(QMap<uint32_t, QByteArray> compressedData);

    void doFlashing();
    void prepareFlashing();
    void startFlashing();
    void requestDownload();
    void transferData();
    void validateFlashing();
    void finishFlashing();

signals:

    /**
     * @brief Signals that DEBUG text is available for printing to console
     * @param text To be printed
     */
    void debugPrint(const QString &text);

    /**
     * @brief Signals that INFO text is available for printing to console
     * @param text To be printed
     */
    void infoPrint(const QString &text);

    /**
     * @brief Signals that ERROR text is available for printing to console
     * @param text To be printed
     */
    void errorPrint(const QString &text);

    /**
     * @brief Signals that the text for flashing need to be changed (Mainwindow GUI)
     * @param s Status to be used
     * @param str Text to be printed
     * @param percent 0..100 to be set as bar value
     */
    void updateStatus(FlashManager::STATUS s, const QString &str, int percent);

    /**
     * @brief Signals that Start of Flashing Thread was requested
     */
    void flashingStartThreadRequested();

    /**
     * @brief Signals that Stop of Flashing Thread was requested
     */
    void flashingStopThreadRequested();

    /**
     * @brief Signals that Flashing Thread was started
     */
    void flashingThreadStarted();

    /**
     * @brief Signals that Flashing Thread was stopped
     */
    void flashingThreadFinished();

public slots:

    /**
     * @brief Slot to run the flashing Thread
     */
    void runThread();

    /**
     * @brief Slot to forward to infoPrint
     * @param text
     */
    void forwardToConsole(const QString &text);

};


#endif /* FLASHMANAGER_H_ */
