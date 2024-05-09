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

    connect(ui->button_choose, &QPushButton::clicked, this, [=]() {
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

    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, [=]() {
        QTableWidgetItem *item = ui->tableWidget->selectedItems().at(0);
            ui->label_2->setText("Selected: " + item->text());
    });
    connect(ui->pushButton, &QPushButton::clicked, this, [=]() {
        if(ui->label_2->text() != "")
            dummy_flash(ui->tableWidget->selectedItems().at(0)->text());
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
