#include <QFileDialog>
#include <QThread>
#include <QDebug>

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


    tests = new Testcases();

    // GUI Console Print
    connect(tests, SIGNAL(toConsole(QString)), this->ui->consoleOut, SLOT(appendPlainText(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_StartSelfTest_clicked()
{

    this->ui->consoleOut->clear();
    // Start GUI Test
    this->ui->consoleOut->appendPlainText("Starting Self Tests\n\tMake sure that the CAN Interface is connected to Virtual CAN Bus (Vector Hardware Manager)\n");

    tests->setTestMode(Testcases::SELFTEST); // Selftests
    tests->startTests();
}


void MainWindow::on_StartECUTest_clicked()
{
    this->ui->consoleOut->clear();
    // Start ECU Test
    this->ui->consoleOut->appendPlainText("Starting ECU Tests\n\tMake sure that the CAN Interface is connected to CAN Bus with ECU connected (Vector Hardware Manager)\n");


    tests->setTestMode(Testcases::MCUTEST);
    tests->startTests();
}


void MainWindow::on_StartUDSListening_clicked()
{
    this->ui->consoleOut->clear();
    // Start UDS Listening Mode
    this->ui->consoleOut->appendPlainText("Starting UDS Listening Mode\n\tMake sure that the CAN Interface is connected a CAN network that contains the UDS Message to be received\n");

    tests->setTestMode(Testcases::LISTENING);
}


void MainWindow::on_ECUISOTPTx_clicked()
{
    this->ui->consoleOut->clear();
    this->ui->consoleOut->appendPlainText("Send some ISO TP Frames to ECU\n\tMake sure that the CAN Interface is connected a CAN network that contains the UDS Message to be received\n");

    tests->setTestMode(Testcases::MCUISOTP);
    tests->startTests();
}

