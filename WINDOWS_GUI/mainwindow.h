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
    Ui::MainWindow *ui;
    EditableComboBox *editComboBox_speed;
    QComboBox *comboBox_speedUnit;

    Communication *comm;
    UDS *uds;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateStatus(MainWindow::status s, QString str);

private:
    void connectSignalSlots();

private slots:
    void comboBoxIndexChanged(int index);
    void appendTextToConsole(const QString &text);
};
#endif // MAINWINDOW_H
