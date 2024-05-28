// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : testcase.hpp
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Base class for specific Testcase
//============================================================================

#ifndef TESTCASE_H_
#define TESTCASE_H_

#include <QObject>
#include <QByteArray>

#include <QDebug>

#include "../WINDOWS_GUI/UDS_Layer/UDS.hpp"
#include "../WINDOWS_GUI/Communication_Layer/Communication.hpp"

class Testcase : public QObject{
    Q_OBJECT

protected:
    uint32_t ecu_id;
    uint8_t gui_id, no_gui_id;

    Communication *comm;
    UDS *uds;

public:
    Testcase(uint8_t gui_id);
    virtual ~Testcase();

    // Validation/RX Method
    virtual void messageChecker(const unsigned int id, const QByteArray &rec);

    // TX Method
    virtual void startTests();

protected:
    uint32_t createCommonID(uint32_t base_id, uint8_t gui_id, uint32_t ecu_id);
    uint8_t checkEqual(unsigned int recid, const QByteArray &rec, unsigned int checkid, QByteArray &check);


signals:
    /**
     * @brief Signals a Text to be print to GUI console
     */
    void toConsole(const QString &);

public slots:
    /**
     * @brief Slot for received UDS Message to be checked
     * @param id ID of Sender
     * @param ba ByteArray with received data
     */
    void rxDataReceiverSlot(const unsigned int id, const QByteArray &ba);

    /**
     * @brief Slot for forwarding a Text to be print to GUI console
     */
    void consoleForward(const QString &console);

};
#endif /* TESTCASE_H_ */
