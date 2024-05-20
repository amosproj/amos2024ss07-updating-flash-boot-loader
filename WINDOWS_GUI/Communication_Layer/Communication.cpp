// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : Communication.cpp
// Author      : Michael Bauer
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
    curr_interface_type = 0; // Initial with Virtual Driver

    multiframe_curr_id = 0; // Init receiving ID
    multiframe_curr_uds_msg = NULL;
    multiframe_curr_uds_msg_len = 0;
    multiframe_next_msg_available = 0;
    multiframe_still_receiving = 0;


    // The thread and the worker are created in the constructor so it is always safe to delete them.
    threadVD = new QThread();
    virtualDriver = new VirtualDriver(); // Initialize Virtual Driver
    virtualDriver->setInterfaceID(0);
    virtualDriver->moveToThread(threadVD);
    connect(virtualDriver, SIGNAL(rxStartThreadRequested()), threadVD, SLOT(start()));
    connect(threadVD, SIGNAL(started()), virtualDriver, SLOT(runThread()));
    connect(virtualDriver, SIGNAL(rxThreadFinished()), threadVD, SLOT(quit()), Qt::DirectConnection);

    threadCAN = new QThread();
    canDriver = new CAN_Wrapper(500000);
    canDriver->setInterfaceID(1);
    canDriver->moveToThread(threadCAN);
    connect(canDriver, SIGNAL(rxStartThreadRequested()), threadCAN, SLOT(start()));
    connect(threadCAN, SIGNAL(started()), canDriver, SLOT(runThread()));
    connect(canDriver, SIGNAL(rxThreadFinished()), threadCAN, SLOT(quit()), Qt::DirectConnection);
}

Communication::~Communication() {
    virtualDriver->stopRX();
    canDriver->stopRX();
}

void Communication::init(uint8_t comm_interface_type){

	uint8_t init_status = 0;

	if(comm_interface_type == COMM_INTERFACE_VIRTUAL){ // Init VirtualDriver
        init_status = virtualDriver->initDriver();
        // TODO: Connect Virtual Driver RX with Communication RX
        //connect(virtualDriver, SIGNAL(rxDataReceived(unsigned int, QByteArray)), this, SLOT(rxCANDataSlot(unsigned int, QByteArray)), Qt::DirectConnection);

        // Connect Communication TX with Virtual Driver TX
        connect(this, SIGNAL(txVirtualDataSignal(QByteArray)), virtualDriver, SLOT(txDataSlot(QByteArray)), Qt::DirectConnection);
        virtualDriver->startRX();
	}
	else if(comm_interface_type == COMM_INTERFACE_CAN){ // Init CanDriver
        init_status = canDriver->initDriver();
        // Connect CAN Driver RX with Communication RX
        connect(canDriver, SIGNAL(rxDataReceived(unsigned int, QByteArray)), this, SLOT(rxCANDataSlot(unsigned int, QByteArray)), Qt::DirectConnection);

        // Connect Communication TX with CAN Driver TX
        connect(this, SIGNAL(txCANDataSignal(QByteArray)), canDriver, SLOT(txDataSlot(QByteArray)), Qt::DirectConnection);

        canDriver->startRX();
	}

	// TODO: Error Handling
	if(!init_status){
        qInfo() << "Communication: Init successful for type " << comm_interface_type;
		return;
	}
    qInfo() << "Communication: Error with init of driver with type " << comm_interface_type;
	return;
}

void Communication::setCommunicationType(uint8_t comm_interface_type){

	this->curr_interface_type = comm_interface_type;
    qInfo() << "Communication: Set interface to type " << comm_interface_type;
}

void Communication::setID(uint32_t id){
	if(curr_interface_type == COMM_INTERFACE_VIRTUAL){ // VirtualDriver
        virtualDriver->setID(id);
	}
	else if(curr_interface_type == COMM_INTERFACE_CAN){ // CANDriver
        canDriver->setID(id);
	}
}

void Communication::txData(uint8_t *data, uint32_t no_bytes){

	if(curr_interface_type == COMM_INTERFACE_VIRTUAL){ // Using Virtual Driver
        qInfo("Communication: Sending out Data via Virtual Driver interface");
		uint8_t *send_msg;
		int send_len;
		int has_next;
		//int max_len_per_frame = 8; // Also use CAN Message Length
		uint8_t max_len_per_frame = 8; // Also use CAN Message Length
		uint32_t data_ptr = 0;
		uint8_t idx = 0;

		send_msg = tx_starting_frame(&send_len, &has_next, max_len_per_frame, data, no_bytes, &data_ptr);
        // Wrap data into QByteArray for signaling
        QByteArray qbdata;
        qbdata.resize(send_len);
        for(int i=0; i < qbdata.size(); i++)
            qbdata[i] = send_msg[i];
        // Free the allocated memory of msg
        free(send_msg);

        qInfo("Communication: Sending Signal txVirtualDataSignal with payload (Single/First Frame)");
        emit txVirtualDataSignal(qbdata);

		if (has_next){ // Check in flow control and continue sending
			// TODO: Wait on flow control...

			while(has_next){
				send_msg = tx_consecutive_frame(&send_len, &has_next, max_len_per_frame, data, no_bytes, &data_ptr, &idx);

                // Wrap data into QByteArray for signaling
                qbdata.clear();
                qbdata.resize(send_len);
                for(int i=0; i < qbdata.size(); i++)
                    qbdata[i] = send_msg[i];
                // Free the allocated memory of msg
                free(send_msg);

                qInfo("Communication: Sending Signal txVirtualDataSignal with payload (Consecutive Frame)");
                emit txVirtualDataSignal(qbdata);
			}
		}
	}

	else if(curr_interface_type == COMM_INTERFACE_CAN){ // Using CAN
        qInfo("Communication: Sending out Data via CAN Driver - Started!");
		uint8_t *send_msg;
		int send_len;
		int has_next;
		uint8_t max_len_per_frame = 8; // CAN Message Length
		uint32_t data_ptr = 0;
		uint8_t idx = 0;

		send_msg = tx_starting_frame(&send_len, &has_next, max_len_per_frame, data, no_bytes, &data_ptr);
        // Wrap data into QByteArray for signaling
        QByteArray qbdata;
        qbdata.resize(send_len);
        for(int i=0; i < qbdata.size(); i++)
            qbdata[i] = send_msg[i];
        // Free the allocated memory of msg
        free(send_msg);

        qInfo("Communication: Sending Signal txCANDataSignal with payload (Single/First Frame)");
        emit txCANDataSignal(qbdata);

		if (has_next){ // Check in flow control and continue sending
			// TODO: Wait on flow control...

			while(has_next){
				send_msg = tx_consecutive_frame(&send_len, &has_next, max_len_per_frame, data, no_bytes, &data_ptr, &idx);
                // Wrap data into QByteArray for signaling
                qbdata.clear();
                qbdata.resize(send_len);
                for(int i=0; i < qbdata.size(); i++)
                    qbdata[i] = send_msg[i];
                // Free the allocated memory of msg
                free(send_msg);

                qInfo("Communication: Sending Signal txCANDataSignal with payload (Consecutive Frame)");
                emit txCANDataSignal(qbdata);
			}
		}
	}
    qInfo("Communication: Sending out Data via CAN Driver - Finished!");
}

void Communication::dataReceiveHandleMulti(){

    if(multiframe_still_receiving == 1 && multiframe_next_msg_available == 0 && multiframe_curr_uds_msg != NULL){
        QByteArray ba;
        ba.resize(multiframe_curr_uds_msg_len);
        for(int i = 0; i < multiframe_curr_uds_msg_len; i++)
            ba[i] = multiframe_curr_uds_msg[i];
        const unsigned int id_ba = multiframe_curr_id;

        // Emit Signal
        qInfo("Communication: Sending Signal rxDataReceived for Multi Frame");
        emit rxDataReceived(id_ba, ba);

        // Reset both receiving flags and ID
        multiframe_still_receiving = 0;
        multiframe_curr_id = 0;

        multiframe_curr_uds_msg_len = 0;
        multiframe_curr_uds_msg = NULL;
    }
}


void Communication::handleCANEvent(unsigned int id, unsigned short dlc, unsigned char data[]){

	// Real processing
    if(curr_interface_type != COMM_INTERFACE_CAN) // CAN is not allowed to forward messages
		return;

    if(dlc == 0){ // Ignoring Empty Messages
        return;
    }

	uint8_t starting_frame = rx_is_starting_frame(data, dlc, MAX_FRAME_LEN_CAN);
	if(starting_frame){
        int temp_uds_msg_len = 0;
        int temp_next_msg_available = 0;
        uint8_t* temp_uds_msg = rx_starting_frame(&temp_uds_msg_len, &temp_next_msg_available, MAX_FRAME_LEN_CAN, data, dlc);

        if(!temp_next_msg_available){ // Single Frame
            QByteArray ba;
            ba.resize(temp_uds_msg_len);
            for(int i = 0; i < temp_uds_msg_len; i++)
                ba[i] = temp_uds_msg[i];
            const unsigned int id_ba = id;

            // Emit Signal
            qInfo("Communication: Sending Signal rxDataReceived for Single Frame");
            emit rxDataReceived(id_ba, ba);
        }

        else {
            if(multiframe_curr_id != 0 && id != multiframe_curr_id){ // Ignore other IDs
                qInfo()<<"Communication: Ignoring ID"<<id<<". Still processing communication with "<<multiframe_curr_id;
                return;
            }
            //qInfo("Call of Starting Frame\n");

            multiframe_still_receiving = 1;
            multiframe_curr_id = id;
            multiframe_curr_uds_msg = temp_uds_msg;
            multiframe_curr_uds_msg_idx = 6; // First 6 bytes contained in First Frame
            multiframe_curr_uds_msg_len = temp_uds_msg_len;
            multiframe_next_msg_available = temp_next_msg_available;
        }
        return;
    }

	uint8_t consecutive_frame = rx_is_consecutive_frame(data, dlc, MAX_FRAME_LEN_CAN);
	if(consecutive_frame){
        if(multiframe_curr_id != 0 && id != multiframe_curr_id){ // Ignore other IDs
            qInfo()<<"Communication: Ignoring ID"<<id<<". Still processing communication with "<<multiframe_curr_id;
            return;
        }
        //qInfo()<<"Call of Consecutive Frame: DLC "<<dlc;

        multiframe_still_receiving = 1;
        rx_consecutive_frame(&multiframe_curr_uds_msg_len, multiframe_curr_uds_msg, &multiframe_next_msg_available, dlc, data, &multiframe_curr_uds_msg_idx);
        this->dataReceiveHandleMulti();
        return;
	}
}

void Communication::setTestMode(){
    if(curr_interface_type == COMM_INTERFACE_VIRTUAL){ // Virtual Driver
        // No changes for Testing necessary
    }
    else if(curr_interface_type == COMM_INTERFACE_CAN){ // CAN Driver
        canDriver->setTestingAppname();
    }
}

//============================================================================
// Slots
//============================================================================

void Communication::rxCANDataSlot(const unsigned int id, const QByteArray &ba){
    qInfo("Communication: Slot - Received RX CAN Data to be processed");
    uint8_t* data = (uint8_t*)calloc(ba.size(), sizeof(uint8_t));
    if(data != nullptr){
        for(int i = 0; i < ba.size(); i++){
            data[i] = ba[i];
        }
        this->handleCANEvent(id, sizeof(data), data);
        free(data);
    }
}

void Communication::txDataSlot(const QByteArray &data){
    qInfo() << "Communication: Slot - Received TX Data to be transmitted - Size =" << data.size() << " Bytes";

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
    qInfo("Communication: Slot - Received setID");
    this->setID(id);
}
