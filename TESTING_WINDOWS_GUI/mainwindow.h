#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

private slots:
    void on_StartGUITest_clicked();

private:
    Ui::MainWindow *ui;

    // Background Thread
    QThread *thread;

    Communication *comm;
    UDS *uds;
};

#endif // MAINWINDOW_H
