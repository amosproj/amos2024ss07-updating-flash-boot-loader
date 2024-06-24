// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>
// SPDX-FileCopyrightText: 2024 Sebastian Rodriguez <r99@melao.de>

#include <QFileDialog>
#include <QThread>
#include <QAction>
#include <QMessageBox>
#include <QPixmap>
#include <QTableView>
#include <QTimer>

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "./UDS_Spec/uds_comm_spec.h"
#include "editableComboBox.h"

static inline void dummy_function(QByteArray data) {
    qDebug() << "Received " << data;
}

void MainWindow::set_uds_connection(enum UDS_CONN conn){


    // Only consider GUI, Flashing is connected within FlashManager (own UDS instance)
    switch(conn){
        case GUI:
            qDebug("MainWindow: Call of set_uds_connection for GUI");
            // Comm RX Signal to UDS RX Slot
            disconnect(comm, SIGNAL(rxDataReceived(uint, QByteArray)), 0, 0); // disconnect everything connect to rxDataReived

            // UDS TX Signals to Comm TX Slots
            disconnect(uds, SIGNAL(setID(uint32_t)), 0, 0);
            disconnect(uds, SIGNAL(txData(QByteArray)), 0, 0);

            // UDS Receive Signals to GUI Slots
            disconnect(uds, SIGNAL(ecuResponse(QMap<QString,QString>)), 0, 0);

            // ####################################################################################

            // Comm RX Signal to UDS RX Slot
            connect(comm, SIGNAL(rxDataReceived(uint, QByteArray)), uds, SLOT(rxDataReceiverSlot(uint, QByteArray)), Qt::DirectConnection);

            // UDS TX Signals to Comm TX Slots
            connect(uds, SIGNAL(setID(uint32_t)),    comm, SLOT(setIDSlot(uint32_t)), Qt::DirectConnection);
            connect(uds, SIGNAL(txData(QByteArray)), comm, SLOT(txDataSlot(QByteArray)), Qt::DirectConnection);

            // UDS Receive Signals to GUI Slots
            connect(uds, SIGNAL(ecuResponse(QMap<QString,QString>)), this, SLOT(ecuResponseSlot(QMap<QString,QString>)), Qt::DirectConnection);
            break;

        default:
            break;
    }
}

void MainWindow::connectSignalSlots() {

    set_uds_connection(GUI);

    // ValidateManager Signals
    connect(validMan, SIGNAL(infoPrint(QString)), this, SLOT(appendTextToConsole(QString)));
    connect(validMan, SIGNAL(debugPrint(QString)), this, SLOT(appendTextToConsole(QString)));
    connect(validMan, SIGNAL(errorPrint(QString)), this, SLOT(appendTextToConsole(QString)));
    connect(validMan, SIGNAL(updateLabel(ValidateManager::LABEL, QString)), this, SLOT(updateLabelSlot(ValidateManager::LABEL, QString)));

    // FlashManager Signals
    connect(flashMan, SIGNAL(flashingThreadStarted()), this, SLOT(stopUDSUsage()));
    connect(flashMan, SIGNAL(flashingThreadFinished()), this, SLOT(startUDSUsage()));
    connect(flashMan, SIGNAL(flashingStartThreadRequested()), threadFlashing, SLOT(start()));
    connect(threadFlashing, SIGNAL(started()), flashMan, SLOT(runThread()));
    connect(flashMan, SIGNAL(flashingThreadFinished()), threadFlashing, SLOT(quit()), Qt::DirectConnection);
    connect(flashMan, SIGNAL(infoPrint(QString)), this, SLOT(appendTextToConsole(QString)));
    connect(flashMan, SIGNAL(debugPrint(QString)), this, SLOT(appendTextToConsole(QString)));
    connect(flashMan, SIGNAL(errorPrint(QString)), this, SLOT(appendTextToConsole(QString)));
    connect(flashMan, SIGNAL(updateStatus(FlashManager::STATUS, QString, int)), this, SLOT(updateStatusSlot(FlashManager::STATUS, QString, int)));

    // Connect the currentIndexChanged signal of the first QComboBox to the slot comboBoxIndexChanged
    connect(ui->comboBox_channel, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &MainWindow::comboBoxIndexChanged);

    // GUI Console Print
    connect(uds, SIGNAL(toConsole(QString)), this->ui->Console, SLOT(appendPlainText(QString)), Qt::DirectConnection);
    connect(comm, SIGNAL(toConsole(QString)), this, SLOT(appendTextToConsole(QString)), Qt::DirectConnection);

    // GUI menu bar
    connect(ui->menuLicenseQT, &QAction::triggered, this, [=]() {
        QMessageBox::about(nullptr, "QT license",
                       "The app was developed with usage of QT Open Source under LGPLv3.\nThe license can be found in file \"LGPLv3\".\n\n");
    });
    connect(ui->menuLicenseMIT, &QAction::triggered, this, [=]() {
        QMessageBox::about(nullptr, "Code license",
                       "Our code was developed under MIT license.");
    });

    // GUI choose file
    connect(ui->button_file, &QPushButton::clicked, this, [=]() {
        QString path = QFileDialog::getOpenFileName(nullptr, "Choose File");
        if(!path.isEmpty()) {
            QFile file(path);
            if(!file.open(QFile::ReadOnly)) {
                qDebug() << "Couldn't open file " + path + " " + file.errorString();
                return;
            }
            ui->label_size->setText("File size:  " + QString::number(file.size()));
            QByteArray data = file.readAll();
            ui->label_content->setText("File content:  " + data.left(16).toHex());

            // Set file type
            QFileInfo fileInfo(path);
            QString fileType = fileInfo.suffix();

            ui->label_type->setText("File type:  " + fileType);

            // Validate file, result is already prepared for furhter calculations
            validMan->data = validMan->validateFile(data);
            validMan->fileChecksum = validMan->calculateFileChecksum(validMan->data);
            appendTextToConsole(QString::number(validMan->fileChecksum, 16));


            //dummy_function(data);
            file.close();
        }
    });

    // GUI select ECU
    connect(ui->table_ECU, &QTableWidget::itemSelectionChanged, this, [=]() {
        if(ECUSelected()) {
            QTableWidgetItem *item = ui->table_ECU->selectedItems().at(0);
            ui->label_selected_ECU->setText("Selected: " + item->text());
        } else { 
            ui->label_selected_ECU->setText("");
        }
    });

    // GUI reset
    connect(ui->button_reset, &QPushButton::clicked, this, [=]() {
        if(!ECUSelected()) 
            return;
        ui->label_reset_status->setText("Reset status: In progress");
        if(uds->ecuReset(getECUID(), FBL_ECU_RESET_SOFT) == UDS::TX_RX_OK)
            ui->label_reset_status->setText("Reset status: Succeeded");
        else
            ui->label_reset_status->setText("Reset status: Failed");
    });

    // GUI flash
    connect(ui->button_flash, &QPushButton::clicked, this, [=]() {
        this->ui->textBrowser_flash_status->clear();
        this->ui->progressBar_flash->setValue(0);

        if(ui->label_selected_ECU->text() != "") {
            uint32_t selectedID = getECUID();

            if(ui->button_flash->text().contains("Stop")){
                flashMan->stopFlashing();
                threadFlashing->wait();
            }
            else{
                if(validMan != nullptr && validMan->data.size() > 0){
                    flashMan->setFlashFile(validMan->data);
                } else {
                    //flashMan->setTestFile();
                    this->ui->textBrowser_flash_status->setText("No valid Flash File selected");
                }

                flashMan->startFlashing(selectedID, gui_id, comm);
            }

        } else {
            this->ui->textBrowser_flash_status->setText("No valid ECU selected");
        }
    });

    // GUI connectivity indicator
    ecuConnectivityTimer = new QTimer(this);
    connect(ecuConnectivityTimer, SIGNAL(timeout()), this, SLOT(checkECUconnectivity()));
    ecuConnectivityTimer->start(1000);

    //Set baudrate
    connect(this, SIGNAL(baudrateSignal(uint, uint)), comm, SLOT(setBaudrate(uint, uint)), Qt::DirectConnection);

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->geometry().width(),this->geometry().height());

    this->setWindowIcon(QIcon::fromTheme("FlashBootloader",
                                         QIcon(":/application/images/icon.png")));
    const QPixmap pix(":/application/images/logo.png");
    pix.scaled(100,100, Qt::KeepAspectRatio);
    ui->label_logo->setPixmap(pix);

    qInfo("Main: Create Communication Layer");
    threadComm = new QThread();
    comm = new Communication();

    qInfo("Main: Create UDS Layer and connect Communcation Layer to it");
    uds = new UDS(gui_id);

    qInfo("Main: Create the FlashManager");
    threadFlashing = new QThread();
    flashMan = new FlashManager();
    flashMan->moveToThread(threadFlashing);

    qInfo("Main: Create the ValidateManager");
    validMan = new ValidateManager();
    validMan->uds = uds;

    ui->table_ECU->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_ECU->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->table_ECU->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connectSignalSlots();

    // Init the Communication - Need to be after connectSignalsSlots to directly print to console
    comm->setCommunicationType(Communication::CAN_DRIVER); // Set to CAN
    comm->init(Communication::CAN_DRIVER); // Set to CAN
    comm->moveToThread(threadComm);

    // Create both QComboBoxes for later
    editComboBox_speed = new EditableComboBox(this);
    comboBox_speedUnit = new QComboBox(this);

    // Call comboBoxIndexChanged to set up editComboBox_speed initially
    comboBoxIndexChanged(ui->comboBox_channel->currentIndex());

    // Initially hide the other QComboBoxes
    editComboBox_speed->hide();
    comboBox_speedUnit->hide();

    connect(editComboBox_speed, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::setBaudrate);

    ui->Console->setReadOnly(true);

    // Rename Flash Button to make sure that naming is correct
    setFlashButton(FLASH);
}

MainWindow::~MainWindow()
{

    flashMan->stopFlashing();
    threadFlashing->wait();

    delete uds;
    delete comm;
    delete flashMan;
    delete ui;
}


void MainWindow::updateStatus(FlashManager::STATUS s, QString str, int percent) {
    QString status = "";
    int val = min(max(0, percent), 100);
    switch(s) {
        case FlashManager::UPDATE:
            status = "[UPDATE] ";
            this->ui->progressBar_flash->setValue(val);
            break;
        case FlashManager::INFO:
            status = "[INFO] ";
            break;
        case FlashManager::ERR:
            status = "[ERROR] ";
            break;
        case FlashManager::RESET:
            status = "";
            this->ui->progressBar_flash->setValue(0);
            this->ui->textBrowser_flash_status->setText("");
            break;
        default:
            qDebug() << "Error wrong status for updateStatus " + QString::number(val);
            break;
    }

    if(!str.isEmpty()){
        QString rest = this->ui->textBrowser_flash_status->toPlainText();
        this->ui->textBrowser_flash_status->setText(status + str + "\n" + rest);
    }
}


void MainWindow::updateLabel(ValidateManager::LABEL s, QString str) {
    QString status = "";

    switch(s) {
    case ValidateManager::HEADER:
        ui->label_header->setText(str);
        break;
    case ValidateManager::VALID:
        ui->label_valid->setText(str);
        break;
    case ValidateManager::CONTENT:
        ui->label_content->setText(str);
        break;
    case ValidateManager::SIZE:
        ui->label_size->setText(str);
        break;
    case ValidateManager::TYPE:
        ui->label_type->setText(str);
        break;
    default:
        qDebug() << "Error wrong status for updateLabel " + QString::number(s);
        break;
    }
}


void MainWindow::updateECUList(){
    qInfo() << "Updating ECU List";
    //this->ui->table_ECU->clearContents();
    //this->ui->table_ECU->setRowCount(0);
    eculist.clear();
    clearECUTableView();
    uds->reqIdentification();

    // Wait and then check on all the received ECUs
    QTimer::singleShot(500, [this]{
        qInfo("Updating ECU Listing Table");
        for(QString ID : eculist.keys()){
            unsigned int id_int = ID.toUInt();
            uint32_t ecu_id = (0xFFF0 & id_int) >> 4;
            if(id_int > 0){
                uds->readDataByIdentifier(ecu_id, (uint16_t) FBL_DID_SYSTEM_NAME);
                uds->readDataByIdentifier(ecu_id, (uint16_t) FBL_DID_PROGRAMMING_DATE);
            }
        }

        // Short break to process the incoming signals
        QTimer::singleShot(100, [this]{
            updateECUTableView(eculist);
        });
    });
}

void MainWindow::clearECUTableView(){
    // Clear the content, keep the current table layout with rowcount
    for(int row = 0; row < ui->table_ECU->rowCount(); row++){
        for(int col = 0; col < ui->table_ECU->columnCount(); col++){
            QTableWidgetItem *item = new QTableWidgetItem("");
            ui->table_ECU->setItem(row, col, item);
        }
    }
}

void MainWindow::updateECUTableView(QMap<QString, QMap<QString, QString>> eculist){

    clearECUTableView();

    // Expect cleared Table
    int ctr = 0;
    for(QString ID : eculist.keys()){
        uint32_t id_int = ID.toUInt();
        if(id_int > 0 && ui->table_ECU->columnCount() >= 3){
            QTableWidgetItem *ecu = ui->table_ECU->item(ctr, 0);
            ecu->setText(QString("0x%1").arg(id_int, 8, 16, QLatin1Char( '0' )));

            QTableWidgetItem *system_name = ui->table_ECU->item(ctr, 1);
            system_name->setText(eculist[ID][QString::number(FBL_DID_SYSTEM_NAME)]);

            QTableWidgetItem *programming_date = ui->table_ECU->item(ctr, 2);
            programming_date->setText(eculist[ID][QString::number(FBL_DID_PROGRAMMING_DATE)]);
        }
        ctr++;
        if(ctr >= ui->table_ECU->rowCount()){
            appendTextToConsole("Currently max "+QString::number(ui->table_ECU->rowCount()) + " ECUs possible");
            break;
        }
    }
}

uint32_t MainWindow::getECUID() {
    QStringList separated = ui->label_selected_ECU->text().split(": 0x");
    QString ID_HEX = separated[1];
    bool ok;
    return (0xFFF0 & ID_HEX.toUInt(&ok, 16)) >> 4;
}

bool MainWindow::ECUSelected() {
    return !ui->table_ECU->selectedItems().empty() && !ui->table_ECU->selectedItems().at(0)->text().isEmpty();
}

void MainWindow::setFlashButton(FLASH_BTN m){

    switch(m){
        case FLASH:
            ui->button_flash->setText("Flash");
            break;
        case STOP:
            ui->button_flash->setText("Stop Flashing");
            break;
        default:
            ui->button_flash->setText("Flash");
            break;
    }
}

//=============================================================================
// Slots
//=============================================================================

void MainWindow::startUDSUsage(){
    // Connect UDS and Comm Layer
    set_uds_connection(GUI);

    // Connect the Connectivity Timer
    connect(ecuConnectivityTimer, SIGNAL(timeout()), this, SLOT(checkECUconnectivity()));

    // Enable all buttons with UDS messages
    ui->button_reset->setDisabled(false);
    ui->pushButton_ECU_refresh->setDisabled(false);

    // Update of ECU List
    this->updateECUList();

    // Also set the Flash Button
    setFlashButton(FLASH);
}

/**
 * @brief This methods stops all UDS activity in MainWindow class.
 * This is necessary to make sure FlashManager and MainWindow are not using UDS communication at the same time. (Can cause conflicts because of different Threads)
 */
void MainWindow::stopUDSUsage(){
    // Disconnect the Connectivity Timer
    disconnect(ecuConnectivityTimer, SIGNAL(timeout()), this, SLOT(checkECUconnectivity()));

    // Disable all buttons with UDS messages
    ui->button_reset->setDisabled(true);
    ui->pushButton_ECU_refresh->setDisabled(true);

    // Also set the Flash Button
    setFlashButton(STOP);
}

void MainWindow::updateStatusSlot(FlashManager::STATUS s, const QString &str, int percent) {
    //qInfo() << "MainWindow: updateStatusSlot " << s << str << percent;
    this->updateStatus(s, str, percent);
}

void MainWindow::updateLabelSlot(ValidateManager::LABEL s, const QString &str) {
    qDebug() << "MainWindow: updateStatusSlot " << s << str;
    this->updateLabel(s, str);
}

//Will show/hide the new ComboBoxes below the ComboBox for the protocol
void MainWindow::comboBoxIndexChanged(int index)
{
    // Clear the items of the second QComboBox
    editComboBox_speed->clear();
    comboBox_speedUnit->clear();

    // Check if the index corresponds to the desired options
    if (index == 1 || index == 2 || index == 3)
    {
        // Populate the second QComboBox based on the selected index of the first QComboBox
        if (index == 1) // Example condition, replace with your own logic
            editComboBox_speed->addItems({ "33.3", "50", "83.3", "100", "125", "250", "500", "1000"});
        else if (index == 2) // Example condition, replace with your own logic
            editComboBox_speed->addItems({"1000", "2000", "3000", "4000", "5000", "6000", "7000", "8000"});
        else if (index == 3) // Example condition, replace with your own logic
            editComboBox_speed->addItems({"Option A", "Option B", "Option C"});

        comboBox_speedUnit->addItem("kBit/s");

        // Show the second QComboBox
        editComboBox_speed->show();
        comboBox_speedUnit->show();

        // Add the second QComboBox to the layout or widget where you want it to appear
        // For example:
        ui->horizontalLayout_channel->addWidget(editComboBox_speed);
        ui->horizontalLayout_channel->addWidget(comboBox_speedUnit);
    }
    else
    {
        // If index doesn't correspond to desired options, hide the second QComboBox
        editComboBox_speed->hide();
        comboBox_speedUnit->hide();
    }
}

void MainWindow::setBaudrate() {
    unsigned int baudrate = editComboBox_speed->currentNumber();
    unsigned int commType = ui->comboBox_channel->currentIndex();

    emit baudrateSignal(baudrate, commType);
}

// Will write Text to console
void MainWindow::appendTextToConsole(const QString &text){
    ui->Console->appendPlainText(text);
}


void MainWindow::ecuResponseSlot(const QMap<QString, QString> &data){
    for (QString key : data.keys()){
        QStringList ID_SID = key.split("#");
        QString ID, SID;
        if (ID_SID.size() == 2){
            ID = ID_SID[0];
            SID = ID_SID[1];
        }

        QString content = data[key];
        qInfo() << ">> MainWindow: ECU Response: ID(int)=" << ID << " - SID(int)=" << SID << " - Data="<< content;

        if(SID == QString::number(FBL_TESTER_PRESENT)){
            eculist[ID] = QMap<QString, QString>();
        }
        else if(SID == QString::number(FBL_READ_DATA_BY_IDENTIFIER)){
            QStringList DID_Payload = content.split("#");
            QString DID, payload;
            if(DID_Payload.size() == 2){
                DID = DID_Payload[0];
                payload = DID_Payload[1];
                eculist[ID][DID] = payload;
            }
        }
        else if(SID == QString::number(FBL_ECU_RESET)){
            qInfo() << ID << "successfully reset";
        }
        else{
            qInfo() << "SID is not yet supported";
        }
    }
}

void MainWindow::on_pushButton_ECU_refresh_clicked()
{
    updateECUList();
}


void MainWindow::on_clearConsoleButton_clicked()
{
    this->ui->Console->clear();
}

void MainWindow::checkECUconnectivity() {
    QString color = "transparent";
    if(ECUSelected()) {
        auto response = uds->testerPresentResponse(getECUID());
        if(response == UDS::TX_RX_OK)
            color = "green";
        else
            color = "red";
    }
    ui->label_ECU_status->setStyleSheet("QLabel {border-radius: 5px;  max-width: 10px; max-height: 10px; background-color: " + color + "}");
}


