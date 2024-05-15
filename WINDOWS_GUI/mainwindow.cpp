#include <QFileDialog>

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "can_wrapper.hpp"
#include "can_wrapper_event.hpp"

static inline void dummy_function(QByteArray data) {
    qDebug() << "Received" << data;
}

static CAN_Wrapper can = CAN_Wrapper(500000);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << qApp->applicationDirPath();

    this->setWindowIcon(QIcon::fromTheme("FlashBootloader",
                                         QIcon(qApp->applicationDirPath() + "/../../icon.png")));

    connect(ui->button_choose, &QPushButton::clicked, [&]() {
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

    boolean init = can.initDriver();

    if (!init)
    {
        return;
    }

    unsigned int txID = 1;
    can.setID(txID);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_button_can_message_clicked()
{
    byte data[] = {0,1,0,1};
    can.txCAN(data, 4);
}

