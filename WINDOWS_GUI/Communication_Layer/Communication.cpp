// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>
// SPDX-FileCopyrightText: 2024 Wiktor Pilarczyk <wiktorpilar99@gmail.com>

//============================================================================
// Name        : Communication.cpp
// Author      : Michael Bauer Wiktor Pilarczyk
// Version     : 0.2
// Copyright   : MIT
// Description : Qt Communication Layer implementation
//============================================================================

#if defined(_Windows) || defined(_MSC_VER) || defined (__GNUC__)
 #define  STRICT
 #include <windows.h>
#endif

#include "Communication.hpp"
#include "../UDS_Spec/uds_comm_spec.h"

Communication::Communication(){
    curr_interface_type = CAN_DRIVER; // Initial with Virtual Driver

    multiframe_curr_id = 0; // Init receiving ID
    multiframe_curr_uds_msg = NULL;
    multiframe_curr_uds_msg_len = 0;
    multiframe_next_msg_available = 0;
    multiframe_still_receiving = 0;

    threadCAN = new QThread();
    canDriver = new CAN_Wrapper(500000);
    canDriver->setInterfaceID(1);
    canDriver->moveToThread(threadCAN);
    connect(canDriver, SIGNAL(rxStartThreadRequested()), threadCAN, SLOT(start()));
    connect(threadCAN, SIGNAL(started()), canDriver, SLOT(runThread()));
    connect(canDriver, SIGNAL(rxThreadFinished()), threadCAN, SLOT(quit()), Qt::DirectConnection);
    connect(canDriver, SIGNAL(infoPrint(QString)), this, SLOT(consoleForwardInfo(QString)), Qt::DirectConnection);
    connect(canDriver, SIGNAL(debugPrint(QString)), this, SLOT(consoleForwardDebug(QString)), Qt::DirectConnection);
    connect(canDriver, SIGNAL(errorPrint(QString)), this, SLOT(consoleForwardError(QString)), Qt::DirectConnection);
}

Communication::~Communication() {
    canDriver->stopRX();
    threadCAN->wait();

    // Disconnect everything
    disconnect(canDriver, nullptr, nullptr, nullptr);
    disconnect(threadCAN, nullptr, nullptr, nullptr);

    delete canDriver;

    qInfo() << "Communication: Destructor finished";
}

//////////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////////

/**
 * @brief Method to initialize a given Interface Type
 * @param comm_interface_type
 */
void Communication::init(INTERFACE comm_interface_type){

	uint8_t init_status = 0;

    if(comm_interface_type == COMM_INTERFACE_CAN){ // Init CanDriver
        init_status = canDriver->initDriver();
        // Connect CAN Driver RX with Communication RX
        connect(canDriver, SIGNAL(rxDataReceived(unsigned int, QByteArray)), this, SLOT(rxCANDataSlot(unsigned int, QByteArray)), Qt::DirectConnection);

        // Connect Communication TX with CAN Driver TX
        connect(this, SIGNAL(txCANDataSignal(QByteArray)), canDriver, SLOT(txDataSlot(QByteArray)), Qt::DirectConnection);

        canDriver->setFilterMask((uint32_t)(FBLCAN_BASE_ADDRESS) | 0xFFF0); // Only accept responses from valid ECUs
        canDriver->startRX();
	}

	// TODO: Error Handling
	if(!init_status){
        if(VERBOSE_COMMUNICATION) qInfo() << "Communication: Init successful for type " << comm_interface_type;
		return;
	}
    qInfo() << "Communication: Error with init of driver with type " << comm_interface_type;
	return;
}

/**
 * @brief Method to set a specific Communication interface to be used
 * @param comm_interface_type
 */
void Communication::setCommunicationType(INTERFACE comm_interface_type){

	this->curr_interface_type = comm_interface_type;
    qInfo() << "Communication: Set interface to type " << comm_interface_type;
}

/**
 * @brief Method to set the Test Mode for the currently set Communication interface - Used for Testing only
 */
void Communication::setTestMode(){
    if(curr_interface_type == CAN_DRIVER){ // CAN Driver
        canDriver->setTestingAppname();
    }
}

//============================================================================
// Private
//============================================================================

/**
 * @brief Method to set the Target ID of the currently set Communication interface
 * @param id
 */
void Communication::setID(uint32_t id){
    if(curr_interface_type == CAN_DRIVER){ // CANDriver
        canDriver->setID(id);
    }
}

/**
 * @brief Method to transmit data via the currently set Communication interface
 * @param data Data to be transmitted
 * @param no_bytes Number of bytes of the given data
 */
void Communication::txData(uint8_t *data, uint32_t no_bytes) {
    if(curr_interface_type == CAN_DRIVER) {
        uint32_t send_len;
        uint32_t has_next;
        uint8_t max_len_per_frame = MAX_FRAME_LEN_CAN; // Also use CAN Message Length
        uint32_t data_ptr = 0;
        uint8_t idx = 0;
        uint8_t *send_msg = tx_starting_frame(&send_len, &has_next, max_len_per_frame, data, no_bytes, &data_ptr);
        // Wrap data into QByteArray for signaling
        QByteArray qbdata;
        qbdata.resize(send_len);
        for(int i=0; i < qbdata.size(); i++)
            qbdata[i] = send_msg[i];
        // Free the allocated memory of msg
        free(send_msg);
        if(VERBOSE_COMMUNICATION) qInfo("Communication TX: Sending out Data via CAN Driver - Started!");
        if(VERBOSE_COMMUNICATION) qInfo("Communication TX: Sending Signal txCANDataSignal with payload (Single/First Frame)");
        emit txCANDataSignal(qbdata);
        if (has_next) { // Check in flow control and continue sending
            // TODO: Wait on flow control...

            while(has_next) {
                send_msg = tx_consecutive_frame(&send_len, &has_next, max_len_per_frame, data, no_bytes, &data_ptr, &idx);
                // Wrap data into QByteArray for signaling
                qbdata.clear();
                qbdata.resize(send_len);
                for(int i=0; i < qbdata.size(); i++)
                    qbdata[i] = send_msg[i];
                // Free the allocated memory of msg
                free(send_msg);

                if(VERBOSE_COMMUNICATION) qInfo("Communication TX: Sending Signal txCANDataSignal with payload (Consecutive Frame)");
                emit txCANDataSignal(qbdata);
            }
        }
    }
    if(VERBOSE_COMMUNICATION) qInfo("Communication TX: Sending out Data via CAN Driver - Finished!");
}


/**
 * @brief Internal Method to process ISO TP data for Multiframe Data (Starting Frame + Consecutive Frames)
 */
void Communication::dataReceiveHandleMulti(){

    if(multiframe_still_receiving && !multiframe_next_msg_available && multiframe_curr_uds_msg != NULL){
        QByteArray ba;
        ba.resize(multiframe_curr_uds_msg_len);
        for(unsigned int i = 0; i < multiframe_curr_uds_msg_len; i++)
            ba[i] = multiframe_curr_uds_msg[i];
        const unsigned int id_ba = multiframe_curr_id;

        // Debugging
        _debug_printf_isotp_buffer();

        // Emit Signal
        if(VERBOSE_COMMUNICATION) qInfo("Communication RX: Sending Signal rxDataReceived for Multi Frame");
        emit rxDataReceived(id_ba, ba);

        // Reset both receiving flags and ID
        multiframe_still_receiving = 0;
        multiframe_curr_id = 0;

        multiframe_curr_uds_msg_len = 0;
        multiframe_curr_uds_msg = NULL;
    }
}

/**
 * @brief Internal Method to process the RX data of the CAN Driver. Is used by the rxCANDataSlot
 * @param id Sender ID
 * @param dlc Sender Data Length Code of the data
 * @param data Sender Data
 */
void Communication::handleCANEvent(unsigned int id, unsigned short dlc, unsigned char data[]){
    if(dlc == 0){ // Ignoring Empty Messages
        return;
    }

    uint8_t starting_frame = rx_is_starting_frame(data, dlc, MAX_FRAME_LEN_CAN);
    if(starting_frame){
        uint32_t temp_uds_msg_len = 0;
        uint32_t temp_next_msg_available = 0;
        uint8_t* temp_uds_msg = rx_starting_frame(&temp_uds_msg_len, &temp_next_msg_available, MAX_FRAME_LEN_CAN, data, dlc);

        if(!temp_next_msg_available){ // Single Frame
            QByteArray ba;
            ba.resize(temp_uds_msg_len);
            for(unsigned int i = 0; i < temp_uds_msg_len; i++)
                ba[i] = temp_uds_msg[i];
            const unsigned int id_ba = id;

            // Emit Signal
            if(VERBOSE_COMMUNICATION) qInfo("Communication RX: Sending Signal rxDataReceived for Single Frame");
            emit rxDataReceived(id_ba, ba);
        }

        else {
            if(multiframe_curr_id != 0 && id != multiframe_curr_id){ // Ignore other IDs
                if(VERBOSE_COMMUNICATION) qInfo()<<"Communication RX: Ignoring ID"<<id<<". Still processing communication with "<<multiframe_curr_id;
                emit toConsole("Communication RX: Ignoring ID" + QString("0x%1").arg(id, 8, 16, QLatin1Char( '0' )) + ". Still processing communication with " + QString("0x%1").arg(multiframe_curr_id, 8, 16, QLatin1Char( '0' )));
                return;
            }
            //qInfo("Call of Starting Frame\n");
            if(VERBOSE_COMMUNICATION) qInfo("Communication RX: Found ISO-TP First Frame. Waiting to receive other Frames");

            multiframe_still_receiving = 1;
            multiframe_curr_id = id;
            multiframe_curr_uds_msg = temp_uds_msg;
            multiframe_curr_uds_msg_idx = 6; // First 6 bytes contained in First Frame
            multiframe_curr_uds_msg_len = temp_uds_msg_len;
            multiframe_next_msg_available = temp_next_msg_available;

            // Debugging
            _debug_printf_isotp_buffer();
        }
        return;
    }

    uint8_t consecutive_frame = rx_is_consecutive_frame(data, dlc, MAX_FRAME_LEN_CAN);
    if(consecutive_frame){
        if(multiframe_curr_id && id != multiframe_curr_id){ // Ignore other IDs
            if(VERBOSE_COMMUNICATION) qInfo()<<"Communication RX: Ignoring ID"<<id<<". Still processing communication with "<<multiframe_curr_id;
            emit toConsole("Communication RX: Ignoring ID" + QString("0x%1").arg(id, 8, 16, QLatin1Char( '0' )) + ". Still processing communication with " + QString("0x%1").arg(multiframe_curr_id, 8, 16, QLatin1Char( '0' )));
            return;
        }
        if(VERBOSE_COMMUNICATION) qInfo() << "Communication RX: Found ISO-TP Consecutive Frame with DLC "<<dlc;

        multiframe_still_receiving = 1;
        rx_consecutive_frame(&multiframe_curr_uds_msg_len, multiframe_curr_uds_msg, &multiframe_next_msg_available, dlc, data, &multiframe_curr_uds_msg_idx);
        this->dataReceiveHandleMulti();
        return;
    }
}

/**
 * @brief Internal Method to print the current ISO TP buffer content
 */
void Communication::_debug_printf_isotp_buffer(){
    if(multiframe_curr_uds_msg != NULL && multiframe_curr_uds_msg_len > 0){
        QString s = "Communication RX: Current ISO-TP Data:";
        for(unsigned int i = 0; i < multiframe_curr_uds_msg_len; i ++){
            s.append(" "+ QString("%1").arg(uint8_t(multiframe_curr_uds_msg[i]), 2, 16, QLatin1Char( '0' )));
        }

        s.append(" - IDX: "+ QString::number(multiframe_curr_uds_msg_idx));

        if(VERBOSE_COMMUNICATION) qInfo() << s.trimmed().toStdString();
    }
}

//============================================================================
// Slots
//============================================================================

void Communication::rxCANDataSlot(const unsigned int id, const QByteArray &ba){
    // Real processing
    if(curr_interface_type != CAN_DRIVER) // CAN_DRIVER message are ignored if a diff interface is selected
        return;

    if(VERBOSE_COMMUNICATION) qInfo("Communication RX: Slot - Received RX CAN Data to be processed");
    uint8_t* data = (uint8_t*)calloc(ba.size(), sizeof(uint8_t));
    if(data != nullptr){
        QString bytes_data = "";
        for(int i = 0; i < ba.size(); i++){
            data[i] = ba[i];
            bytes_data.append(QString("%1").arg(uint8_t(data[i]), 2, 16, QLatin1Char( '0' )) + " ");

        }
        if(VERBOSE_COMMUNICATION) qInfo() << "Communication RX: rxCANDataSlot extracted data"<<bytes_data.trimmed()<<" from ID"<<QString("0x%1").arg(id, 8, 16, QLatin1Char( '0' ));
        this->handleCANEvent(id, ba.size(), data);
        free(data);
    }
}

void Communication::txDataSlot(const QByteArray &data){
    if(VERBOSE_COMMUNICATION) qInfo() << "Communication TX: Slot - Received TX Data to be transmitted - Size =" << data.size() << " Bytes";

    // Unwrap the received data
    uint8_t* msg = (uint8_t*)calloc(data.size(), sizeof(uint8_t));
    if (msg != nullptr){
        for(int i= 0; i < data.size(); i++){
            msg[i] = data[i];
            //qInfo() << "Step " << i << "Data: " << msg[i];
        }
        this->txData(msg, data.size());
        free(msg);
    }
}

void Communication::setIDSlot(uint32_t id){
    if(VERBOSE_COMMUNICATION) qInfo("Communication TX: Slot - Received setID");
    this->setID(id);
}

void Communication::setBaudrate(unsigned int baudrate, unsigned int commType) {
    if (commType == 1) { //CAN
        connect(this, SIGNAL(baudrateSignal(unsigned int)), canDriver, SLOT(setChannelBaudrate(unsigned int)), Qt::DirectConnection);
        emit baudrateSignal(baudrate);
        disconnect(this, SIGNAL(baudrateSignal(unsigned int)), canDriver, SLOT(setChannelBaudrate(unsigned int)));
    } else if (commType == 2) { //CAN-FD
        //TODO has to be changed as soon as we have CAN-FD implemented
    } else if (commType == 3){ //ethernet
        //need commInterface instance for default implementation
    }
}

//============================================================================
// Private Slots
//============================================================================
void Communication::consoleForwardInfo(const QString &text){
    if(VERBOSE_COMMUNICATION) qInfo()<< "Communication: Slot Received consoleForwardInfo:" << text;
    emit toConsole("Info: "+text);
}

void Communication::consoleForwardDebug(const QString &text){
    if(VERBOSE_COMMUNICATION) qInfo() << "Communication: Slot Received consoleForwardDebug:" << text;
    emit toConsole("Debug: "+text);
}

void Communication::consoleForwardError(const QString &text){
    if(VERBOSE_COMMUNICATION) qInfo() << "Communication: Slot Received consoleForwardError:" << text;
    emit toConsole("Error: "+text);
}
