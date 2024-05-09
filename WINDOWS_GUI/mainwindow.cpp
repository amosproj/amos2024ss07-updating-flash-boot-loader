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
        if(ui->label_selected_ECU->text() != "")
            dummy_flash(ui->table_ECU->selectedItems().at(0)->text());
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
