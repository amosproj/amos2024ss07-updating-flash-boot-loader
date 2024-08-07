// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define ECU_CONNECTIVITY_TIMER      (1000) // ms between UDS tester present msg -> Default: 1000

#include <QMainWindow>
#include <QMutex>
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
    QWidget resetToBootloaderPopup;
    QString rootDir;
    QString defaultRootDir;

    QThread *threadComm;
    Communication *comm;
    UDS *uds;
    QThread *threadFlashing;
    FlashManager *flashMan;
    QMutex ecuListUpdateMutex;
    bool ecuListUpdateInProgess;
    QMap<QString, QMap<QString, QString>> eculist;
    bool validManagerValuesAvailable;
    ValidateManager *validMan;
    QTimer *ecuConnectivityTimer;

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateStatus(FlashManager::STATUS s, QString str, int percent);
    void updateLabel(ValidateManager::LABEL s, QString str);


private:
    void set_uds_connection(enum UDS_CONN);
    void connectSignalSlots();
    void setupFlashPopup();
    void setupResetToBootloaderPopup();

    void updateECUList();
    void clearECUTableView();
    void updateECUTableView(QMap<QString, QMap<QString, QString>> eculist);

    uint32_t getECUID();
    QString getECUHEXID();
    bool ECUSelected();

    void setFlashButton(FLASH_BTN m);

    bool updateValidManager();

private slots:
    void startUDSUsage();
    void stopUDSUsage();

    void updateStatusSlot(FlashManager::STATUS s, const QString &str, int percent);
    void updateLabelSlot(ValidateManager::LABEL s, const QString &str);

    void comboBoxIndexChanged(int index);

    void setBaudrate();

    void appendTextToConsole(const QString &text);



    void ecuResponseSlot(const QMap<QString, QString> &data);
    void on_pushButton_ECU_refresh_clicked();
    void on_clearConsoleButton_clicked();
    void checkECUconnectivity();


    void onValidationDone(const QMap<uint32_t, QByteArray> result);

signals:
    void baudrateSignal(unsigned int baudrate, unsigned int commType);


};
#endif // MAINWINDOW_H
