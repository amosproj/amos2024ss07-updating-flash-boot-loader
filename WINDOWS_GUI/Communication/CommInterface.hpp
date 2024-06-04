// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : Comminterface.h
// Author      : Michael Bauer
// Version     : 0.2
// Copyright   : MIT
// Description : Communication Interface Implementation
//============================================================================

#ifndef COMMUNICATION_LAYER_COMMINTERFACE_H_
#define COMMUNICATION_LAYER_COMMINTERFACE_H_

#define VERBOSE_COMMINTERFACE   0   // switch for verbose console information

#include <QObject>
#include <QMutex>
#include <QByteArray>

#include <stdint.h>

class CommInterface : public QObject{
    Q_OBJECT

protected:
    uint8_t type;       // 0 = Virtual Driver, 1 = CAN, 2 = CAN_FD
    uint8_t own_id;     // ID to identify the interface

    uint32_t id;        // ID for transmitting

    bool _abort;        // Thread Handling
    bool _working;      // Thread Handling
    QMutex mutex;       // Protects _abort and _working

public:
    explicit CommInterface(QObject *parent = 0);
	virtual ~CommInterface();

	uint8_t getType(){
		return this->type;
	}

	uint8_t getInterfaceID(){
		return this->own_id;
	}

	void setInterfaceID(uint8_t id){
		this->own_id = id;
	}

    void startRX(){
        mutex.lock();
        _working = true;
        _abort = false;
        mutex.unlock();
        emit rxStartThreadRequested();
    }

    void stopRX(){
        mutex.lock();
        if (_working) {
            _abort = true;
        }
        mutex.unlock();
        emit rxStopThreadRequested();
    }

protected:
	virtual void setID(uint32_t id){
		this->id = id;
	}

	virtual uint8_t initDriver();
    virtual uint8_t txData(uint8_t *data, uint8_t no_bytes);

    virtual void doRX();

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
     * @brief Signals the status of driver init
     * @param initstatus
     */
    void driverInit(const QString &initstatus);

    /**
     * @brief Signals that TX was requested
     */
    void txDataSentRequested(const QString &request);

    /**
     * @brief Signals the status if data was sent or not
     */
    void txDataSentStatus(const QString &status);

    /**
     * @brief Signals that Start of RX Thread was requested
     */
    void rxStartThreadRequested();

    /**
     * @brief Signals that Stop of RX Thread was requested
     */
    void rxStopThreadRequested();

    /**
     * @brief Signals that RX Thread was stopped
     */
    void rxThreadFinished();

    /**
     * @brief Signals that RX Data is received
     * @param id ID of the Sender
     * @param ba ByteArray with the data
     */
    void rxDataReceived (const unsigned int id, const QByteArray &ba );

public slots:

    /**
     * @brief Slot to run the RX Thread
     */
    void runThread();

    /**
     * @brief Slot to send Data via Interface
     * @param data ByteArray with data to be transmitted
     */
    void txDataSlot(const QByteArray &data);

};

#endif /* COMMUNICATION_LAYER_COMMINTERFACE_H_ */
