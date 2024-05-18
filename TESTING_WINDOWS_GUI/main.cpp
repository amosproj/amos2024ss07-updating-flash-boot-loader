#include <QApplication>
#include <QMessageBox>

#include "mainwindow.h"

#include <stdio.h>
#include "../WINDOWS_GUI/Communication_Layer/Communication.h"
#include "../WINDOWS_GUI/UDS_Layer/UDS.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMessageBox::about(nullptr, "License", 
                       "The app was developed with usage of QT Open Source under LGPLv3.\nThe license can be found in file \"LGPLv3\".");

    printf("Main: Create Communication Layer\n");
    Communication comm = Communication();
    comm.setCommunicationType(1); // Set to CAN
    comm.setTestMode(); // Only necessary for Testing!
    comm.init(1); // Set to CAN

    printf("Main: Create UDS Layer and connect Communcation Layer to it\n");
    UDS uds = UDS(0x001, &comm);
    comm.setUDSInterpreter(&uds);

    MainWindow w;
    w.show();
    return a.exec();
}
