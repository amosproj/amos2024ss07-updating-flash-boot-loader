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

#define VERBOSE_FLASHMANAGER   0   // switch for verbose console information
#define MAX_TRIES_PER_STATE    5   // Max Attempts per state
#define WAITTIME_AFTER_ATTEMPT  1000 // Waittime in ms

#include <QObject>
#include <QMutex>
#include <QDebug>

#include <stdint.h>

#include "UDS_Layer/UDS.hpp"

class FlashManager : public QObject {
    Q_OBJECT

public:
    enum STATUS {UPDATE, INFO, ERR, RESET };

private:
    enum STATE_MACHINE {PREPARE, EXECUTE, FINISH, IDLE, ERR_STATE};
    STATE_MACHINE curr_state, prev_state;           // States
    uint8_t state_attempt_ctr;                      // State attempt counter
    uint32_t ecu_id;                                // ECU ID to flash
    UDS *uds;                                       // Reference to UDS Layer
    QString file;                                   // Reference to file for flashing

    bool _abort;                                    // Thread Handling
    bool _working;                                  // Thread Handling
    QMutex mutex;                                   // Protects _abort and _working

public:
    explicit FlashManager(QObject *parent = 0);
    virtual ~FlashManager();

    void setUDS(UDS *uds);
    void setECUID(uint32_t ecu_id);
    void setFile(QString file);

    void startFlashing(uint32_t ecu_id){

        if(ecu_id <= 0){
            emit errorPrint("FlashManager: Could not start flashing. Wrong ECU ID given");
            return;
        }

        this->ecu_id = ecu_id;

        state_attempt_ctr = 0;
        prev_state = IDLE;
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

        qInfo() << "FlashManager: Stopping Flashing Thread";
        emit flashingStopThreadRequested();
    }

private:
    QByteArray getCurrentFlashDate();

    void doFlashing();
    void prepareFlashing();
    void executeFlashing();
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

};


#endif /* FLASHMANAGER_H_ */
