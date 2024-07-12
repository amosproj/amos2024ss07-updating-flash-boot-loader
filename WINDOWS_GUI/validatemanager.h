// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Leon Wilms <leonwilms.wk@gmail.com>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : validatemanager.h
// Author      : Leon Wilms, Michael Bauer
// Version     : 0.2
// Copyright   : MIT
// Description : Validation Manager to validate selected files
//============================================================================

#ifndef VALIDATEMANAGER_H
#define VALIDATEMANAGER_H

#define MINIMUM_BLOCK_SIZE          (32)    // Bytes, Content of 1 Page
#define ADD_SUPPORTING_PAGES_EVERY  0x50000  // Number of bytes if there is a big gap between two addresses within range

#include <QObject>
#include <QDebug>
#include <QMap>
#include <QByteArray>
#include <QThread>
#include <QMutex>

class ValidateManager : public QObject {

    Q_OBJECT

public:

    enum LABEL {HEADER, VALID, CONTENT, SIZE, TYPE};

    QMap<uint32_t, QByteArray> data;

private:

    QMutex dataMutex;
    QMap<uint16_t, QMap<QString, QString>> core_addr;

public:

    ValidateManager();
    virtual ~ValidateManager();

    void setCoreAddr(QMap<uint16_t, QMap<QString, QString>> new_core_addr);

    void validateFileAsync(QByteArray data);
    bool checkBlockAddressRange(QMap<uint32_t, QByteArray> blocks);

    QMap<uint32_t, QByteArray> transformData(QMap<uint32_t, QByteArray> blocks);

private:

    QMap<uint32_t, QByteArray> validateFile(QByteArray data);

    bool validateLine(QByteArray line);
    QByteArray extractData(QByteArray line, char record_type);
    QMap<uint32_t, QByteArray> combineSortedQMap(QMap<uint32_t, QByteArray> blocks);

    bool addrInCoreRange(uint32_t addr, uint32_t data_len,  uint16_t core, bool* supported);
    bool addrInRange(uint32_t address, uint32_t data_len);


    QByteArray getData(QByteArray tempData);
    uint32_t getAddr(uint32_t addr);

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
     * @brief Signals that the text for validation need to be changed (Mainwindow GUI)
     * @param s Status to be used
     * @param str Text to be printed
     * @param percent 0..100 to be set as bar value
     */
    void updateLabel(ValidateManager::LABEL s, const QString &str);

    /**
     * @brief Signals that ERROR text is available for printing to console
     * @param text To be printed
     */
    void validationDone(const QMap<uint32_t, QByteArray> result);

};


#endif // VALIDATEMANAGER_H
