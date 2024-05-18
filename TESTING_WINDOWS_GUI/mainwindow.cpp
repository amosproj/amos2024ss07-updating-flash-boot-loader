#include <QFileDialog>
#include <QThread>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(1200,600);

    this->setWindowIcon(QIcon::fromTheme("Testing FlashBootloader",
                                         QIcon("../../icon.png")));


}

MainWindow::~MainWindow()
{
    delete ui;
}

