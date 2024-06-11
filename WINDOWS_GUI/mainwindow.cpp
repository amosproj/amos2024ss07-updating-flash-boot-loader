// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>
// SPDX-FileCopyrightText: 2024 Sebastian Rodriguez <r99@melao.de>

#include <QFileDialog>
#include <QThread>
#include <QAction>
#include <QMessageBox>
#include <QPixmap>
#include <QTimer>
#include <QTableView>

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "./UDS_Spec/uds_comm_spec.h"
#include "editableComboBox.h"

static inline void dummy_function(QByteArray data) {
    qDebug() << "Received " << data;
}

static inline void dummy_flash(QString dev) {
    qDebug() << "Flash " << dev;
}

void MainWindow::connectSignalSlots() {
    // Comm RX Signal to UDS RX Slot
    connect(comm, SIGNAL(rxDataReceived(uint, QByteArray)), uds, SLOT(rxDataReceiverSlot(uint, QByteArray)), Qt::DirectConnection);

    // UDS TX Signals to Comm TX Slots
    connect(uds, SIGNAL(setID(uint32_t)),    comm, SLOT(setIDSlot(uint32_t)));
    connect(uds, SIGNAL(txData(QByteArray)), comm, SLOT(txDataSlot(QByteArray)));

    // UDS Receive Signals to GUI Slots
    connect(uds, SIGNAL(ecuResponse(QMap<QString,QString>)), this, SLOT(ecuResponseSlot(QMap<QString,QString>)));

    // Connect the currentIndexChanged signal of the first QComboBox to the slot comboBoxIndexChanged
    connect(ui->comboBox_channel, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &MainWindow::comboBoxIndexChanged);

    // GUI Console Print
    connect(uds, SIGNAL(toConsole(QString)), this->ui->Console, SLOT(appendPlainText(QString)));
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
            ui->label_size->setText("File size: " + QString::number(file.size()));
            QByteArray data = file.readAll();
            ui->label_content->setText("File content: " + data.left(16).toHex());
            dummy_function(data);
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
            dummy_flash(ui->label_selected_ECU->text());
            // Just for demonstration purposes
            updateStatus(RESET, "");
            updateStatus(UPDATE, "Flashing started for " + ui->label_selected_ECU->text());
            updateStatus(INFO, "It may take a while");

            for(int j = 1; j < 100; j++)
                QTimer::singleShot(j*175, [this]{
                    updateStatus(UPDATE, "Flashing in Progress.. Please Wait");
                });
            QTimer::singleShot(100*175, [this]{
                updateStatus(INFO, "Flashing finished!");
            });

            appendTextToConsole("INFO: ONLY DEMO UI - Flashing currently not supported on ECU.");
        } else {
            this->ui->textBrowser_flash_status->setText("No valid ECU selected");
        }
    });

    // GUI connectivity indicator
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkECUconnectivity()));
    timer->start(1000);

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
    comm = new Communication();

    qInfo("Main: Create UDS Layer and connect Communcation Layer to it");
    uds = new UDS(gui_id);

    ui->table_ECU->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_ECU->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->table_ECU->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connectSignalSlots();

    // Init the Communication - Need to be after connectSignalsSlots to directly print to console
    comm->setCommunicationType(Communication::CAN_DRIVER); // Set to CAN
    comm->init(Communication::CAN_DRIVER); // Set to CAN


    // Create both QComboBoxes for later
    editComboBox_speed = new EditableComboBox(this);
    comboBox_speedUnit = new QComboBox(this);

    // Call comboBoxIndexChanged to set up editComboBox_speed initially
    comboBoxIndexChanged(ui->comboBox_channel->currentIndex());

    // Initially hide the other QComboBoxes
    editComboBox_speed->hide();
    comboBox_speedUnit->hide();

    connect(editComboBox_speed, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::editComboBoxIndexChanged);

    ui->Console->setReadOnly(true);
}

MainWindow::~MainWindow()
{
    delete uds;
    delete comm;
    delete ui;
}

void MainWindow::updateStatus(MainWindow::status s, QString str) {
    QString status = "";
    int val = 0;
    switch(s) {
        case UPDATE:
            status = "[UPDATE] ";
            //qDebug() << this->ui->progressBar_flash->value();
            val = this->ui->progressBar_flash->value() + 1;
            if(val % 10)
                status = "";

            if(val <= 100)
                this->ui->progressBar_flash->setValue(val);
            break;
        case INFO:
            status = "[INFO] ";
            break;
        case ERR:
            status = "[ERROR] ";
            break;
        case RESET:
            status = "";
            this->ui->progressBar_flash->setValue(0);
            this->ui->textBrowser_flash_status->setText("");
            break;
        default:
            qDebug() << "Error wrong status for updateStatus " + QString::number(val);
            break;
    }

    if(!status.isEmpty()){
        QString rest = this->ui->textBrowser_flash_status->toPlainText();
        this->ui->textBrowser_flash_status->setText(status + str + "\n" + rest);
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

//=============================================================================
// Slots
//=============================================================================

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
        comboBox_speedUnit->addItem("MBit/s");

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

void MainWindow::editComboBoxIndexChanged(int index) {
    CAN_Wrapper *can = comm->getCANWrapper();
    double bitrate;
    if (comboBox_speedUnit->currentIndex() == 0) {
        bitrate = editComboBox_speed->currentData() * 1000;
    } else if (comboBox_speedUnit->currentIndex() == 1) {
        bitrate = editComboBox_speed->currentData() * 1000000;
    } else {
        bitrate = -1;
    }
    if (bitrate == -1) {
        return;
    }
    XLstatus status = can->setBaudrate(bitrate);
    if (status != XL_SUCCESS) {
        QString errorMsg = "Failed setting bitrate: ";
        errorMsg.append(xlGetErrorString(status));
        appendTextToConsole(errorMsg);
        return;
    }
    QString msg = "Successfully set bitrate to: ";
    msg.append(QString::number(bitrate));
    msg.append(" Bit/s");
    appendTextToConsole(msg);
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
