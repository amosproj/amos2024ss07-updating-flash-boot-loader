// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : consoleUpdater.h
// Author      : Michael Bauer
// Version     : 0.1
// Copyright   : MIT
// Description : Worker Object to update GUI Console
//============================================================================

#ifndef CONSOLE_UPDATER_H
#define CONSOLE_UPDATER_H

#include <QObject>

class ConsoleUpdater : public QObject {
    Q_OBJECT

public:
    explicit ConsoleUpdater(QObject *parent = 0) : QObject(parent) {}
    void appendToConsole(const QString &text) { emit requestAppendingConsole(text); }

signals:
    void requestAppendingConsole(const QString &);
};

#endif // CONSOLE_UPDATER_H
