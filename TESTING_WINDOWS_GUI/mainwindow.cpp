#include <QFileDialog>
#include <QThread>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->geometry().width(),this->geometry().height());


    this->setWindowIcon(QIcon::fromTheme("Testing Flashbootloader",
                                         QIcon("../../icon.png")));

}

MainWindow::~MainWindow()
{
    delete ui;
}

