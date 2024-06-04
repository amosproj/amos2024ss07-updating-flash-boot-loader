// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "editableComboBox.h"

#include "UDS_Layer/UDS.hpp"
#include "Communication_Layer/Communication.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    enum status {
        UPDATE,
        INFO,
        ERR,
        RESET
    };
private:
    uint8_t gui_id = 0x01;

    Ui::MainWindow *ui;
    EditableComboBox *editComboBox_speed;
    QComboBox *comboBox_speedUnit;

    Communication *comm;
    UDS *uds;
    QMap<QString, QMap<QString, QString>> eculist;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateStatus(MainWindow::status s, QString str);

private:
    void connectSignalSlots();
    void updateECUList();

    void clearECUTableView();
    void updateECUTableView(QMap<QString, QMap<QString, QString>> eculist);

private slots:
    void comboBoxIndexChanged(int index);
    void appendTextToConsole(const QString &text);

    void ecuResponseSlot(const QMap<QString, QString> &data);
    void on_pushButton_ECU_refresh_clicked();
};
#endif // MAINWINDOW_H
