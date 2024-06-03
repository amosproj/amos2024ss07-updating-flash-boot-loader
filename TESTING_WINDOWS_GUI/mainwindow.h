// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "testcasecontroller.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startTest_clicked();

    void on_testSelectionBox_currentTextChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
    Testcasecontroller *tests;
};

#endif // MAINWINDOW_H
