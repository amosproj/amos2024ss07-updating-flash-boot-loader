#include <QFileDialog>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

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
