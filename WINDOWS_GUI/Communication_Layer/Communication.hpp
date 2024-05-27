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

#define COMM_INTERFACE_VIRTUAL				(0x0)
#define COMM_INTERFACE_CAN					(0x1)


class Communication : public QObject{
    Q_OBJECT

public:
    enum INTERFACE {VIRTUAL_DRIVER = COMM_INTERFACE_VIRTUAL, CAN_DRIVER = COMM_INTERFACE_CAN};

private:
    QThread *threadVD;                          // Thread for the Virtual Driver
    VirtualDriver* virtualDriver;               // Instance of the Virtual Driver
    QThread *threadCAN;                         // Thread for the CAN Driver
    CAN_Wrapper* canDriver;                     // Instance of the CAN Driver

    INTERFACE curr_interface_type;

    // Used for consecutive frames
    uint32_t multiframe_curr_id;                // ECU ID of the currently processed Multiframe
    uint8_t *multiframe_curr_uds_msg;           // Pointer to currently process UDS Multiframe message
    int multiframe_curr_uds_msg_len;            // Length of the UDS Multiframe message
    uint32_t multiframe_curr_uds_msg_idx;       // UDS Multiframe message index of the data, starting idx for writing to multiframe_curr_uds_msg
    int multiframe_next_msg_available;          // Indicates if next frame is available
    uint8_t multiframe_still_receiving;         // Indicates that Multiframe receiving is still ongoing, used as Trigger for final Frame of the Multiframe message

public:
	Communication();
	~Communication();

    void init(INTERFACE ct);
    void setCommunicationType(INTERFACE ct);
	void setID(uint32_t id); // TODO: Check on Architecture

	void txData(uint8_t *data, uint32_t no_bytes);

    // Testing
    void setTestMode();

private:
    void dataReceiveHandleMulti();

    // CAN Event Received
    void handleCANEvent(unsigned int id, unsigned short dlc, unsigned char data[]);

    void _debug_printf_isotp_buffer();

signals:
    /**
     * @brief Signals that RX Data is received from pre selected interface
     * @param id ID of the Sender
     * @param ba ByteArray with the data
     */
    void rxDataReceived (const unsigned int id, const QByteArray &ba);

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
