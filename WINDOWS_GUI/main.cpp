// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>

#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "mainwindow.h"

//#include <stdio.h>



#include <QMutex>

QFile logFile;

QMutex handlerMutex;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {

    static const char *typeStr[] = {"[DEBUG]   ", "[WARNING] ", "[CRITICAL]", "[FATAL]   ", "[INFO]    "};

    QString logMsg = QString("%1 %2 %3")

                         .arg(QDateTime::currentDateTime().toString("hh:mm:ss dd-MM-yyyy"))

                         .arg(typeStr[type])

                         .arg(msg);

    handlerMutex.lock();

    QTextStream out(&logFile);

    out << logMsg << Qt::endl;

    handlerMutex.unlock();
}

void trimLogFile(const QString &filePath, int maxLines = 5000) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd())
        lines.append(in.readLine());
    file.close();

    if (lines.size() > maxLines) {
        lines = lines.mid(lines.size() - maxLines);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&file);
            for (const QString &line : lines)
                out << line << Qt::endl;
            file.close();
        }
    }
}

int main(int argc, char *argv[])
{
    QString logFilePath = "GUI.log";

    trimLogFile(logFilePath);
    logFile.setFileName(logFilePath);
    if(!logFile.open( QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Log file openning failed";
        return 1;
    }


    qInstallMessageHandler(messageHandler);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
