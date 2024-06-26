// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define ECU_CONNECTIVITY_TIMER      (10000) // ms between UDS tester present msg -> TODO: change back to 1000ms

#include <QMainWindow>
#include "editableComboBox.h"

#include "UDS_Layer/UDS.hpp"
#include "Communication_Layer/Communication.hpp"
#include "flashmanager.h"
#include "validatemanager.h"

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
    enum FLASH_BTN {FLASH, STOP};
    enum UDS_CONN {GUI, FLASHING};

    Ui::MainWindow *ui;
    EditableComboBox *editComboBox_speed;
    QComboBox *comboBox_speedUnit;
    QWidget flashPopup;
    QString filePath;

    QThread *threadComm;
    Communication *comm;
    UDS *uds;
    QThread *threadFlashing;
    FlashManager *flashMan;
    QMap<QString, QMap<QString, QString>> eculist;
    ValidateManager *validMan;
    QTimer *ecuConnectivityTimer;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateStatus(FlashManager::STATUS s, QString str, int percent);
    void updateLabel(ValidateManager::LABEL s, QString str);


private:
    void set_uds_connection(enum UDS_CONN);
    void connectSignalSlots();
    void updateECUList();

    void clearECUTableView();
    void updateECUTableView(QMap<QString, QMap<QString, QString>> eculist);

    uint32_t getECUID();
    QString getECUHEXID();
    bool ECUSelected();

    void updateValidManager();

    void udsUpdateVersion(uint32_t id, uint8_t *data, uint8_t data_size);

    void setupFlashPopup();
    void setFlashButton(FLASH_BTN m);

private slots:
    void startUDSUsage();
    void stopUDSUsage();

    void updateStatusSlot(FlashManager::STATUS s, const QString &str, int percent);

    void comboBoxIndexChanged(int index);
    void appendTextToConsole(const QString &text);

    void updateLabelSlot(ValidateManager::LABEL s, const QString &str);

    void ecuResponseSlot(const QMap<QString, QString> &data);
    void on_pushButton_ECU_refresh_clicked();
    void on_clearConsoleButton_clicked();
    void checkECUconnectivity();
    void setBaudrate();

signals:
    void baudrateSignal(unsigned int baudrate, unsigned int commType);


};
#endif // MAINWINDOW_H
