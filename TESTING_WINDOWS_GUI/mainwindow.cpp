#include <QFileDialog>
#include <QDebug>
#include <QString>

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


    tests = new Testcasecontroller();

    // GUI Console Print
    connect(tests, SIGNAL(toConsole(QString)), this->ui->consoleOut, SLOT(appendPlainText(QString)));

    // Add the different Testcases
    this->ui->testSelectionBox->clear();
    this->ui->testSelectionBox->addItems({"Testcase: UDS Selftest (Testing GUI only)",
                                          "Testcase: Send UDS Messages to ECU (Testing GUI <-> ECU)",
                                          "Testcase: UDS Listening only (ECU/GUI -> Testing GUI)",
                                          "Testcase: Send ISO TP Frames to ECU (Testing GUI -> ECU)"
                                        });

    // Default:
    on_testSelectionBox_currentTextChanged("Testcase: UDS Selftest (Testing GUI only)");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_startTest_clicked()
{
    tests->startTests();
}


void MainWindow::on_testSelectionBox_currentTextChanged(const QString &arg1)
{
    this->ui->consoleOut->clear();
    if(arg1 == "Testcase: UDS Selftest (Testing GUI only)"){
        // Start GUI Test
        this->ui->consoleOut->appendPlainText("Starting Self Tests\n\tMake sure that the CAN Interface is connected to Virtual CAN Bus (Vector Hardware Manager)\n");
        tests->setTestMode(Testcasecontroller::SELFTEST); // Selftests
    }

    else if(arg1 == "Testcase: Send UDS Messages to ECU (Testing GUI <-> ECU)"){
        // Start ECU Test
        this->ui->consoleOut->appendPlainText("Starting ECU Tests\n\tMake sure that the CAN Interface is connected to CAN Bus with ECU connected (Vector Hardware Manager)\n");

        tests->setTestMode(Testcasecontroller::ECUTEST);
    }

    else if(arg1 == "Testcase: UDS Listening only (ECU/GUI -> Testing GUI)"){
        // Start UDS Listening Mode
        this->ui->consoleOut->appendPlainText("Starting UDS Listening Mode\n\tMake sure that the CAN Interface is connected a CAN network that contains the UDS Message to be received\n");

        tests->setTestMode(Testcasecontroller::LISTENING);
    }

    else if(arg1 == "Testcase: Send ISO TP Frames to ECU (Testing GUI -> ECU)"){
        // Start UDS Listening Mode
        this->ui->consoleOut->appendPlainText("Send some ISO TP Frames to ECU\n\tMake sure that the CAN Interface is connected a CAN network that contains the UDS Message to be received\n");

        tests->setTestMode(Testcasecontroller::ECUISOTP);
    }
}

