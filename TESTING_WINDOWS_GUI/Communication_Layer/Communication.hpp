// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : Communication.hpp
// Author      : Michael Bauer
// Version     : 0.2
// Copyright   : MIT
// Description : Qt Communication Layer implementation
//============================================================================

#ifndef COMMUNICATION_LAYER_COMMUNICATION_H_
#define COMMUNICATION_LAYER_COMMUNICATION_H_

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QByteArray>

#include <stdint.h>

#include "../Communication/Can_Wrapper.hpp"
#include "../Communication/VirtualDriver.hpp"
#include "../UDS_Layer/UDSMsg.h"

#define COMM_INTERFACE_VIRTUAL				(0x0)
#define COMM_INTERFACE_CAN					(0x1)


class Communication : public QObject{
    Q_OBJECT

private:
    QThread *threadVD;
    VirtualDriver* virtualDriver;
    QThread *threadCAN;
    CAN_Wrapper* canDriver;

	uint8_t curr_interface_type;

    // Used for consecutive frames
    uint32_t multiframe_curr_id;
    uint8_t *multiframe_curr_uds_msg;
    int multiframe_curr_uds_msg_len;
    uint32_t multiframe_curr_uds_msg_idx;
    int multiframe_next_msg_available;
    uint8_t multiframe_still_receiving;

public:
	Communication();
	~Communication();

	void init(uint8_t ct);
	void setCommunicationType(uint8_t ct);
	void setID(uint32_t id); // TODO: Check on Architecture

	void txData(uint8_t *data, uint32_t no_bytes);

    void dataReceiveHandleMulti();

    // CAN Event Received
    void handleCANEvent(unsigned int id, unsigned short dlc, unsigned char data[]);

    // Testing
    void setTestMode();

signals:
    /**
     * @brief Signals that RX Data is received from pre selected interface
     * @param id ID of the Sender
     * @param ba ByteArray with the data
     */
    void rxDataReceived (const UDS_Msg &uds);

    /**
     * @brief Signals that Virtual TX Data is ready to be transmitted
     * @param data Contains Data to be transmitted
     */
    void txVirtualDataSignal(const QByteArray &data);

    /**
     * @brief Signals that CAN TX Data is ready to be transmitted
     * @param data Contains Data to be transmitted
     */
    void txCANDataSignal(const QByteArray &data);


public slots:

    /**
     * @brief Slot for the CAN Driver
     * @param id
     * @param ba
     */
    void rxCANDataSlot(const unsigned int id, const QByteArray &ba);

    /**
     * @brief Slot to send Data via the pre selected interface
     * @param data Array including the data
     */
    void txDataSlot(const QByteArray &data);

    /**
     * @brief Slot to set the ID for the pre selected interface
     * @param id ID to be set
     */
    void setIDSlot(uint32_t id);

};

#endif /* COMMUNICATION_LAYER_COMMUNICATION_H_ */
