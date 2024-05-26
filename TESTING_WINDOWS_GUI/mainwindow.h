#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "testcases.hpp"

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
    void on_StartSelfTest_clicked();

    void on_StartECUTest_clicked();

    void on_StartUDSListening_clicked();

private:
    Ui::MainWindow *ui;
    Testcases *tests;
};

#endif // MAINWINDOW_H
