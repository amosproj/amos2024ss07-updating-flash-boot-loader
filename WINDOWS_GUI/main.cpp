#include <QApplication>
#include <QMessageBox>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMessageBox::about(nullptr, "License", 
                       "The app was developed with usage of QT Open Source under LGPLv3.\nThe license can be found in file \"LGPLv3\".");
    MainWindow w;
    w.show();
    return a.exec();
}
