//============================================================================
// Name        : validatemanager.cpp
// Author      : Leon Wilms
// Version     : 0.1
// Copyright   : MIT
// Description : Validation Manager to validate selected files
//============================================================================

#ifndef VALIDATEMANAGER_H
#define VALIDATEMANAGER_H

#include <QObject>
#include <QDebug>
#include <QMap>
#include <QByteArray>

#include "UDS_Layer/UDS.hpp"



class ValidateManager : public QObject {

    Q_OBJECT

public:

    ValidateManager();

    enum LABEL {HEADER, VALID, CONTENT, SIZE, TYPE};

    QMap<uint32_t, QByteArray> validateFile(QByteArray data);
    uint32_t calculateFileChecksum(QMap<uint32_t, QByteArray> data);

    QMap<uint32_t, QByteArray> data;
    uint32_t fileChecksum;

    UDS *uds;

private:
         // Add any private member variables or functions here if needed

    bool validateLine(QByteArray line);
    QByteArray extractData(QByteArray line, char record_type);



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
    void updateLabel(ValidateManager::LABEL s, const QString &str);


};


#endif // VALIDATEMANAGER_H
