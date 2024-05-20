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


    qInfo("Main: Create Communication Layer");
    comm = new Communication();
    comm->setCommunicationType(1); // Set to CAN
    comm->init(1); // Set to CAN

    qInfo("Main: Create UDS Layer and connect Communcation Layer to it");
    uds = new UDS(0x001);

    //=====================================================================
    // Connect the signals and slots

    // Comm RX Signal to UDS RX Slot
    connect(comm, SIGNAL(rxDataReceived(unsigned int, QByteArray)), uds, SLOT(rxDataReceiverSlot(unsigned int, QByteArray)), Qt::DirectConnection);

    // UDS TX Signals to Comm TX Slots
    connect(uds, SIGNAL(setID(uint32_t)),    comm, SLOT(setIDSlot(uint32_t)));
    connect(uds, SIGNAL(txData(QByteArray)), comm, SLOT(txDataSlot(QByteArray)));
    //=====================================================================

    // GUI Console Print
    connect(uds, SIGNAL(toConsole(QString)), this->ui->consoleOut, SLOT(appendPlainText(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_StartGUITest_clicked()
{
    uds->testerPresent(0x001);
}
