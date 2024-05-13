#include <QFileDialog>

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "editableComboBox.h"

static inline void dummy_function(QByteArray data) {
    qDebug() << "Received " << data;
}

static inline void dummy_flash(QString dev) {
    qDebug() << "Flash " << dev;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << qApp->applicationDirPath();

    this->setWindowIcon(QIcon::fromTheme("FlashBootloader",
                                         QIcon(qApp->applicationDirPath() + "/../../icon.png")));

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

    ui->table_ECU->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_ECU->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->table_ECU->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui->table_ECU, &QTableWidget::itemSelectionChanged, this, [=]() {
        QTableWidgetItem *item = ui->table_ECU->selectedItems().at(0);
        ui->label_selected_ECU->setText("Selected: " + item->text());
    });
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


    // Create the second QComboBox
    secondComboBox = new EditableComboBox(this);

    // Call comboBoxIndexChanged to set up secondComboBox initially
    comboBoxIndexChanged(ui->comboBox_channel->currentIndex());

    // Initially hide the second QComboBox
    secondComboBox->hide();

    // Connect the currentIndexChanged signal of the first QComboBox to the slot comboBoxIndexChanged
    connect(ui->comboBox_channel, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &MainWindow::comboBoxIndexChanged);

    //Connect the clicked signal of label_test_channel to the slot updateStatusLabel
    connect(ui->button_test_channel, &QPushButton::clicked, this,
            &MainWindow::updateButtonLabel);

}

MainWindow::~MainWindow()
{
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
    case ERROR:
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
    QString rest = this->ui->textBrowser_flash_status->toPlainText();
    this->ui->textBrowser_flash_status->setText(status + str + "\n" + rest);

}
void MainWindow::comboBoxIndexChanged(int index)
{
    // Clear the items of the second QComboBox
    secondComboBox->clear();

    // Check if the index corresponds to the desired options
    if (index == 1 || index == 2 || index == 3)
    {
        // Populate the second QComboBox based on the selected index of the first QComboBox
        if (index == 1) // Example condition, replace with your own logic
        {
            secondComboBox->addItem("Option A");
            secondComboBox->addItem("Option B");
            secondComboBox->addItem("Option C");
        }
        else if (index == 2) // Example condition, replace with your own logic
        {
            secondComboBox->addItem("Option D");
            secondComboBox->addItem("Option E");
            secondComboBox->addItem("Option F");
        }
        else if (index == 3) // Example condition, replace with your own logic
        {
            secondComboBox->addItem("Option X");
            secondComboBox->addItem("Option Y");
            secondComboBox->addItem("Option Z");
        }

        // Show the second QComboBox
        secondComboBox->show();

        // Add the second QComboBox to the layout or widget where you want it to appear
        // For example:
        ui->verticalLayout_channel->insertWidget(2, secondComboBox);
    }
    else
    {
        // If index doesn't correspond to desired options, hide the second QComboBox
        secondComboBox->hide();
    }
}

void MainWindow::updateButtonLabel()
{
    ui->button_test_channel->setText(QString("Edit Mode: %1").arg(secondComboBox->editMode ? "ON" : "OFF"));
}
