// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "editableComboBox.h"

#include "UDS_Layer/UDS.hpp"
#include "Communication_Layer/Communication.hpp"
#include "flashmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    

private:
    uint8_t gui_id = 0x01;

    Ui::MainWindow *ui;
    EditableComboBox *editComboBox_speed;
    QComboBox *comboBox_speedUnit;

    Communication *comm;
    UDS *uds;
    QThread *threadFlashing;
    FlashManager *flashMan;
    QMap<QString, QMap<QString, QString>> eculist;

    QTimer *ecuConnectivityTimer;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateStatus(FlashManager::STATUS s, QString str, int percent);

private:
    void connectSignalSlots();
    void updateECUList();

    void clearECUTableView();
    void updateECUTableView(QMap<QString, QMap<QString, QString>> eculist);

    uint32_t getECUID();
    bool ECUSelected();

private slots:
    void startUDSUsage();
    void stopUDSUsage();

    void updateStatusSlot(FlashManager::STATUS s, const QString &str, int percent);

    void comboBoxIndexChanged(int index);
    void appendTextToConsole(const QString &text);

    void ecuResponseSlot(const QMap<QString, QString> &data);
    void on_pushButton_ECU_refresh_clicked();
    void on_clearConsoleButton_clicked();
    void checkECUconnectivity();
    void setBaudrate();

signals:
    void baudrateSignal(unsigned int baudrate, unsigned int commType);
};
#endif // MAINWINDOW_H
