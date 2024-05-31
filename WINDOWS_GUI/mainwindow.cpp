#include <QFileDialog>
#include <QThread>
#include <QAction>
#include <QMessageBox>
#include <QPixmap>

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
    connect(comm, SIGNAL(rxDataReceived(unsigned int, QByteArray)), uds, SLOT(rxDataReceiverSlot(unsigned int, QByteArray)), Qt::DirectConnection);

    // UDS TX Signals to Comm TX Slots
    connect(uds, SIGNAL(setID(uint32_t)),    comm, SLOT(setIDSlot(uint32_t)));
    connect(uds, SIGNAL(txData(QByteArray)), comm, SLOT(txDataSlot(QByteArray)));

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
        QTableWidgetItem *item = ui->table_ECU->selectedItems().at(0);
        ui->label_selected_ECU->setText("Selected: " + item->text());
    });

    // GUI reset
    connect(ui->button_reset, &QPushButton::clicked, this, [=]() {
        if(ui->label_selected_ECU->text() != "") {
            ui->label_reset_status->setText("Reset status: In progress");
            if(uds->ecuReset(0x001, FBL_ECU_RESET_WARM_POWERON) == UDS::TX_RX_OK) 
                ui->label_reset_status->setText("Reset status: Succeeded");
            else
                ui->label_reset_status->setText("Reset status: Failed");
        }
    });

    // GUI flash
    connect(ui->button_flash, &QPushButton::clicked, this, [=]() {
        if(ui->label_selected_ECU->text() != "") {
            dummy_flash(ui->label_selected_ECU->text());
            // Just for demonstration purposes
            updateStatus(RESET, "");
            updateStatus(UPDATE, "Flashing started for " + ui->label_selected_ECU->text());
            updateStatus(INFO, "It may take a while");
            updateStatus(UPDATE, "Already did X");
        }
    });
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->geometry().width(),this->geometry().height());

    this->setWindowIcon(QIcon::fromTheme("FlashBootloader",
                                         QIcon("../../images/icon.png")));
    const QPixmap pix("../../images/logo.png");
    pix.scaled(100,100, Qt::KeepAspectRatio);
    ui->label_logo->setPixmap(pix);

    qInfo("Main: Create Communication Layer");
    comm = new Communication();

    qInfo("Main: Create UDS Layer and connect Communcation Layer to it");
    uds = new UDS(0x001);

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

    ui->Console->setReadOnly(true);
}

MainWindow::~MainWindow()
{
    delete uds;
    delete comm;
    delete ui;
}

void MainWindow::updateStatus(MainWindow::status s, QString str) {
    QString status;
    int val = 0;
    switch(s) {
        case UPDATE:
            status = "[UPDATE] ";
            qDebug() << this->ui->progressBar_flash->value();
            val = this->ui->progressBar_flash->value() + 10;
            this->ui->progressBar_flash->setValue(val);
            break;
        case INFO:
            status = "[INFO] ";
            break;
        /*case ERROR:
            status = "[ERROR] ";
            break;*/
        case RESET:
            status = "";
            this->ui->progressBar_flash->setValue(0);
            this->ui->textBrowser_flash_status->setText("");
            break;
        default:
            qDebug() << "Error wrong status for updateStatus " + QString::number(val);
            break;
    }
    QString rest = this->ui->textBrowser_flash_status->toPlainText();
    this->ui->textBrowser_flash_status->setText(status + str + "\n" + rest);

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
            editComboBox_speed->addItems({"33.3", "50", "83.3", "100", "125", "250", "500", "1000"});
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

// Will write Text to console
void MainWindow::appendTextToConsole(const QString &text){
    ui->Console->appendPlainText(text);
}
