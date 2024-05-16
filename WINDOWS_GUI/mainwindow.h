#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "editableComboBox.h"

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
        ERROR,
        RESET
    };

    void on_button_can_message_clicked();
    void updateStatus(MainWindow::status s, QString str);

private slots:
    void comboBoxIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    EditableComboBox *editComboBox_speed;
    QComboBox *comboBox_speedUnit;
};
#endif // MAINWINDOW_H
