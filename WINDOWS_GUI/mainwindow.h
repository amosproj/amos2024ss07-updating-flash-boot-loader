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
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    enum status {
        UPDATE,
        INFO,
        ERR,
        RESET
    };

    void display_rcvd_can_message(unsigned int id, unsigned short dlc, unsigned char data[]);
    void on_button_can_message_clicked();
    void updateStatus(MainWindow::status s, QString str);

private slots:
    void comboBoxIndexChanged(int index);
    void appendTextToConsole(const QString &text);
    void resetResponseSlot();

private:
    Ui::MainWindow *ui;
    EditableComboBox *editComboBox_speed;
    QComboBox *comboBox_speedUnit;

    Communication *comm;
    UDS *uds;
};
#endif // MAINWINDOW_H
