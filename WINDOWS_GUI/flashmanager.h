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
#define MAX_TRIES_PER_STATE         5          // Max Attempts per state
#define WAITTIME_AFTER_ATTEMPT      1000        // Waittime in ms

#define TESTFILE_START_ADD          0xA0090000  // Start Address for flashing
#define TESTFILE_BYTES              0x0016FFFF  // ~1.5 MB
#define TESTFILE_PADDING_BYTES      7           // Padding between test data

#include <QObject>
#include <QMutex>
#include <QDebug>

#include <stdint.h>

#include "UDS_Layer/UDS.hpp"
#include "Communication_Layer/Communication.hpp"

class FlashManager : public QObject {
    Q_OBJECT

public:
    enum STATUS {UPDATE, INFO, ERR, RESET };

private:
    enum STATE_MACHINE {PREPARE, START_FLASHING, REQ_DOWNLOAD, TRANSFER_DATA, VALIDATE, FINISH, IDLE, ERR_STATE};
    STATE_MACHINE curr_state, prev_state;                       // States
    uint8_t state_attempt_ctr;                                  // State attempt counter
    uint32_t ecu_id;                                            // ECU ID to flash
    UDS *uds;                                                   // Reference to UDS Layer
    Communication *comm;                                        // Reference to Comm Layer
    QString file;                                               // Reference to file for flashing
    QMap<uint32_t, QByteArray> flashContent;                    // Map with Address -> continous byte array
    QMap<uint32_t, uint32_t> fileChecksums;

    size_t flashedBytesCtr;                                     // Counter for flashed bytes
    uint32_t flashCurrentAdd;                                   // Stores the current address to be flashed
    uint32_t flashCurrentPackages;                              // Stores the current number of packes for flashCurrentAdd;
    uint32_t flashCurrentBufferSize;                            // Stores the current buffer size per
    uint32_t flashCurrentPackageCtr;                            // Stores the current counter of the package

    bool _abort;                                                // Thread Handling
    bool _working;                                              // Thread Handling
    QMutex mutex;                                               // Protects _abort and _working

public:
    explicit FlashManager(QObject *parent = 0);
    virtual ~FlashManager();

    void setECUID(uint32_t ecu_id);
    void setTestFile();
    void setFlashFile(QMap<uint32_t, QByteArray> data);
    void setFileChecksums(QMap<uint32_t, uint32_t> checksums);

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

        // Comm RX Signal to UDS RX Slot
        connect(this->comm, SIGNAL(rxDataReceived(uint, QByteArray)), this->uds, SLOT(rxDataReceiverSlot(uint, QByteArray)), Qt::DirectConnection);

        // UDS TX Signals to Comm TX Slots
        connect(this->uds, SIGNAL(setID(uint32_t)),    this->comm, SLOT(setIDSlot(uint32_t)), Qt::DirectConnection);
        connect(this->uds, SIGNAL(txData(QByteArray)), this->comm, SLOT(txDataSlot(QByteArray)), Qt::DirectConnection);

        // GUI Console Print
        connect(this->uds, SIGNAL(toConsole(QString)), this, SLOT(forwardToConsole(QString)), Qt::DirectConnection);
        connect(this->comm, SIGNAL(toConsole(QString)), this, SLOT(forwardToConsole(QString)), Qt::DirectConnection);

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

        // Comm RX Signal to UDS RX Slot
        disconnect(comm, SIGNAL(rxDataReceived(uint, QByteArray)), 0, 0); // disconnect everything connect to rxDataReived

        // UDS TX Signals to Comm TX Slots
        disconnect(uds, SIGNAL(setID(uint32_t)), 0, 0);
        disconnect(uds, SIGNAL(txData(QByteArray)), 0, 0);

        qInfo() << "FlashManager: Stopping Flashing Thread";
        emit flashingStopThreadRequested();
    }

private:
    QByteArray getCurrentFlashDate();
    size_t getOverallByteSize();

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
