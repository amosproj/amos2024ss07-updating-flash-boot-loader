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
#include <QDate>
#include <QTime>

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
        filePath = QFileDialog::getOpenFileName(nullptr, "Choose File");
        if(!filePath.isEmpty()) {
            QFile file(filePath);
            if(!file.open(QFile::ReadOnly)) {
                qDebug() << "Couldn't open file " + filePath + " " + file.errorString();
                return;
            }
            ui->label_size->setText("File size:  " + QString::number(file.size()/1024) + " KB");
            QByteArray data = file.readAll();
            ui->label_content->setText("File content:  " + data.left(16).toHex());

            // Set file type
            QFileInfo fileInfo(filePath);
            QString fileType = fileInfo.suffix();

            ui->label_type->setText("File type:  " + fileType);

            // Validate file, result is already prepared for furhter calculations
            QMap<uint32_t, QByteArray> result = validateFile(data);

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
        if(ui->label_selected_ECU->text() == "") {
            this->ui->textBrowser_flash_status->setText("No valid ECU selected");
        } else if(filePath.isEmpty()) {
            this->ui->textBrowser_flash_status->setText("Flash file NOT selected");
        } else {
            QLabel* label = qobject_cast<QLabel*>(flashPopup.property("label").value<QObject*>());
            label->setText(QString("You are going to flash from ") + QString(this->ui->table_ECU->selectedItems().at(2)->text())
                                    + QString(" to ") + fileVersion);
            this->flashPopup.show();
        }
    });

    // GUI connectivity indicator
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkECUconnectivity()));
    timer->start(1000);

    //Set baudrate
    connect(this, SIGNAL(baudrateSignal(uint, uint)), comm, SLOT(setBaudrate(uint, uint)), Qt::DirectConnection);

}

void MainWindow::setupFlashPopup() {
    QVBoxLayout *layout = new QVBoxLayout;

    QLabel *labelPopup = new QLabel;
    flashPopup.setProperty("label", QVariant::fromValue(static_cast<QObject*>(labelPopup)));

    QPushButton *buttonYes = new QPushButton("Yes");
    QPushButton *buttonNo = new QPushButton("No");

    
    QObject::connect(buttonYes, &QPushButton::clicked, this, [&]() {
        flashPopup.close();
        this->ui->textBrowser_flash_status->clear();
        this->ui->progressBar_flash->setValue(0);

        uint32_t selectedID = getECUID();

        appendTextToConsole("\n\nINFO: ONLY DEMO UI - Flashing currently not supported on ECU.");
        setupECUForFlashing(selectedID);

        dummy_flash(ui->label_selected_ECU->text());
        // Just for demonstration purposes
        updateStatus(RESET, "");
        updateStatus(UPDATE, "Flashing started for " + ui->label_selected_ECU->text());
        updateStatus(INFO, "It may take a while");

        for(int j = 1; j < 100; j++)
            QTimer::singleShot(j*35, [this]{
                updateStatus(UPDATE, "Flashing in Progress.. Please Wait");
            });
        QTimer::singleShot(100*35, [this, selectedID]{
            updateStatus(RESET, "");
            updateStatus(INFO, "Flashing finished!");
            this->udsUpdateProgrammingDate(selectedID);
            QByteArray arr = fileVersion.toLocal8Bit();
            this->udsUpdateVersion(selectedID, (uint8_t*)arr.data(), arr.size());

            updateStatus(INFO, "Updating ECU List");
            this->updateECUList();
        });
    });

    QObject::connect(buttonNo, &QPushButton::clicked, [&]() {
        flashPopup.close();
    });

    layout->addWidget(labelPopup);
    layout->addWidget(buttonYes);
    layout->addWidget(buttonNo);

    flashPopup.setLayout(layout);
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

    setupFlashPopup();

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

    connect(editComboBox_speed, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::setBaudrate);

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
                uds->readDataByIdentifier(ecu_id, (uint16_t) FBL_DID_APP_ID);
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
        if(id_int > 0 && ui->table_ECU->columnCount() >= 4){
            QTableWidgetItem *ecu = ui->table_ECU->item(ctr, 0);
            ecu->setText(QString("0x%1").arg(id_int, 8, 16, QLatin1Char( '0' )));

            QTableWidgetItem *system_name = ui->table_ECU->item(ctr, 1);
            system_name->setText(eculist[ID][QString::number(FBL_DID_SYSTEM_NAME)]);

            
            QTableWidgetItem *app_version = ui->table_ECU->item(ctr, 2);
            app_version->setText(eculist[ID][QString::number(FBL_DID_APP_ID)]);

            QTableWidgetItem *programming_date = ui->table_ECU->item(ctr, 3);
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

void MainWindow::setupECUForFlashing(uint32_t id){

    appendTextToConsole("Change Session to Programming Session for selected ECU");
    uds->diagnosticSessionControl(id, FBL_DIAG_SESSION_PROGRAMMING);

    appendTextToConsole("TODO: Add Security Access once activated");
}

QByteArray MainWindow::getCurrentFlashDate(){

    QDate date = QDate().currentDate();
    QTime time = QTime().currentTime();
    QString date_str = date.toString("ddMMyy");
    QString time_str = time.toString("HHmmss");
    QString combi = date_str + time_str;
    QByteArray bcd = QByteArray::fromHex(QByteArray(combi.toLocal8Bit()));

    return bcd;
}

void MainWindow::udsUpdateProgrammingDate(uint32_t id){
    QByteArray flashdate = getCurrentFlashDate();
    uint8_t *data = (uint8_t*)flashdate.data();

    uds->writeDataByIdentifier(id, FBL_DID_PROGRAMMING_DATE, data, flashdate.size());
}

void MainWindow::udsUpdateVersion(uint32_t id, uint8_t *data, uint8_t data_size) {
    uds->writeDataByIdentifier(id, FBL_DID_APP_ID, data, data_size);
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

QMap<uint32_t, QByteArray> MainWindow::validateFile(QByteArray data)
{
    QList<QByteArray> lines = data.split('\n');
    int current_index = 0;
    int result_index = 0;

    int count_record = -1;
    int count_lines = 0;
    int nlines = lines.size();

    bool file_validity = true;
    bool file_header = false;

    QByteArray new_line;
    QMap<uint32_t, QByteArray> result;

    // Print each line
    for (QByteArray& line : lines) {

        // remove '\r' at the end of each line
        if(current_index != (nlines - 1)){

            line = line.left(line.size() - 1 );
        }

        char record_type = line.at(1);              // get record type
        line = line.mid(2);                         // Trim Record type for preprocessing

        // Check validity of one line at a time
        if(!validateLine(line))
        {

            file_validity = false;
        }

        // extract header information
        if(record_type == '0'){

            QByteArray header;

            line = line.left(line.size() - 2);
            line = line.right(line.size() - 2);

            file_header = true;


            for (int i = 0; i < line.size(); i += 2)
            {
                // Convert each pair of hex characters to a byte
                QString hexPair = line.mid(i, 2);

                char asciiChar = hexPair.toUShort(NULL, 16); // Convert hexPair to integer

                if(asciiChar != NULL){

                    header.append(asciiChar);
                }

            }

            // Convert QByteArray to QString (assuming it's ASCII)
            fileVersion = QString::fromLatin1(header);
        }
        // preprocess data for flashing and count data records for validation
        else if(record_type == '1' or record_type == '2' or record_type == '3'){

            new_line = extractData(line, record_type);

            result.insert(result_index, new_line);

            result_index += 1;
            count_lines += 1;
        }
        // preprocess data for flashing
        else if(record_type == '7' or record_type == '8' or record_type == '9'){

            new_line = extractData(line, record_type);

            result.insert(result_index, new_line);

            result_index += 1;
        }
        // optional entry, can be used to validate file
        else if(record_type == '5' or record_type == '6'){

            line = line.left(line.size() - 2);
            line = line.right(line.size() - 2);

            if(count_record != -1){

                file_validity = false;
            }

            count_record = line.toInt(NULL, 16);
        }
        // S4 is reserved and should not be used & all other inputs are invalid s19 inputs
        else {

            qDebug() << "There was an error! with the selected file";
        }

        current_index += 1;
    }

    if(file_header != true)
        fileVersion = QString("N/A");

    // Check if count record is present and is correctly set
    if(count_record != count_lines and count_record != -1){

        file_validity = false;
    }

    // Show validity of file in UI
    if(file_validity)
    {

        ui->label_valid->setText("File validity:  Valid");

    }
    else {

        ui->label_valid->setText("File validity:  Not Valid");
    }

    ui->label_version->setText("File version: " + fileVersion);

    return result;
}


bool MainWindow::validateLine(QByteArray line)
{
    int sum = 0;
    int checksum = line.right(2).toInt(NULL, 16); // Extracts the last two characters

    // Remove the checksum from the original line for further processing
    QByteArray trimmedLine = line.left(line.size() - 2);

    // Process the line in pairs of hexadecimal values
    for (int i = 0; i < trimmedLine.size(); i += 2)
    {
        // Extract each pair of characters
        QByteArray hexPair = trimmedLine.mid(i, 2);

        // Convert the hexPair to a hexadecimal value
        bool ok;
        uint32_t hexValue = hexPair.toInt(&ok, 16); // Convert hexPair to ushort (16-bit) integer

        if (!ok)
        {
            // Handle conversion error
            qDebug() << "Error converting hexPair:" << hexPair;

            return false;
        }
        else
        {

            sum += hexValue;
        }

    }

    if(checksum == (0xFF - (sum & 0xFF))){

        return true;
    }

    return false;
}

QByteArray MainWindow::extractData(QByteArray line, char record_type)
{
    // Trim Count
    QByteArray trimmed_line = line.left(line.size() - 2);
    // Trim Checksum
    trimmed_line = trimmed_line.right(trimmed_line.size() - 2);

    // Pad address
    if(record_type == '1' or record_type == '9'){

        trimmed_line = trimmed_line.rightJustified(trimmed_line.size() + 4, '0');
    }
    else if(record_type == '2' or record_type == '8'){

        trimmed_line = trimmed_line.rightJustified(trimmed_line.size() + 2, '0');
    }
    else if(record_type == '3' or record_type == '7'){

        // There is nothing to pad, dont do anything
    }
    // Did we encounter a S-record that is not supposed to be here?
    else{

        qDebug() << "There was an error! with current line!";
    }

    return trimmed_line;
}
