// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>

//============================================================================
// Name        : UDS.hpp
// Author      : Michael Bauer
// Version     : 0.3
// Copyright   : MIT
// Description : Qt UDS Layer implementation
//============================================================================
#ifndef UDS_LAYER_UDS_H_
#define UDS_LAYER_UDS_H_

#define VERBOSE_UDS     0                       // switch for verbose console information

#include <QObject>
#include <QByteArray>
#include <QMutex>

#include "stdint.h"


class UDS : public QObject{
    Q_OBJECT

public:
    enum RESP {NO_INIT, STILL_BUSY, TX_FREE, RX_NO_RESPONSE, RX_ERROR, TX_RX_OK, TX_RX_NOK, TX_OK, RX_NEG_RESP};

private:
    bool synchronized_rx_tx;                    // Flag to enable synchronized mode

    uint8_t gui_id;                             // GUI ID for TX, only set during init
    uint8_t init;                               // Init flag

    // Timeout control
    uint32_t tx_max_waittime_free_tx = 1000;    // ms - Wait time before TX aborts
    uint32_t rx_max_waittime_general = 500;     // ms - Wait time before RX aborts
    uint32_t rx_max_waittime_long    = 2000;    // ms - Long wait time before RX aborts

    bool _comm;                                 // For communication usage, only synchronized TX+RX is possible
    QMutex comm_mutex;                          // Protects _comm

    unsigned int rx_exp_id;                     // ID to be expected for response of TX
    uint8_t *rx_exp_data;                       // Data to be expected from ECU, if possible
    int rx_no_bytes;                            // No bytes to be expected from ECU
    bool rx_msg_valid;                          // Indication of Message Interpreter if UDS Msg was valid
    bool rx_msg_neg_resp;                       // Indication of Negative Response

public:
    UDS();
    UDS(uint8_t gui_id);
	virtual ~UDS();

    // Switch for Synchronous TX/RX vs. Async TX Mode
    void setSyncMode(bool synchronized);

    // Sending out broadcast for tester present
    RESP reqIdentification();

	// Specification for Diagnostic and Communication Management
    RESP diagnosticSessionControl(uint32_t id, uint8_t session);
    RESP ecuReset(uint32_t id, uint8_t reset_type);
    RESP securityAccessRequestSEED(uint32_t id);
    RESP securityAccessVerifyKey(uint32_t id, uint8_t *key, uint8_t key_len);
    RESP testerPresent(uint32_t id);

	// Specification for Data Transmission
    RESP readDataByIdentifier(uint32_t id, uint16_t identifier);
    RESP readMemoryByAddress(uint32_t id, uint32_t address, uint16_t no_bytes);
    RESP writeDataByIdentifier(uint32_t id, uint16_t identifier, uint8_t* data, uint8_t data_len);

	// Specification for Upload | Download
    RESP requestDownload(uint32_t id, uint32_t address, uint32_t no_bytes);
    RESP requestUpload(uint32_t id, uint32_t address, uint32_t no_bytes);
    RESP transferData(uint32_t id, uint32_t address, uint8_t* data, uint8_t data_len);
    RESP requestTransferExit(uint32_t id, uint32_t address);

	// Supported Common Response Codes
    RESP negativeResponse(uint32_t id, uint8_t rej_sid, uint8_t neg_resp_code);


private:
    void messageInterpreter(unsigned int id, uint8_t *data, uint32_t no_bytes);

    const RESP txMessageStart();
    void txMessageSend(uint32_t id, uint8_t *msg, int len);
    const RESP txMessageValid();
    RESP checkOnFreeTX();
    RESP checkOnResponse(uint32_t waittime);
	uint32_t createCommonID(uint32_t base_id, uint8_t gui_id, uint32_t ecu_id);
    QString translateNegResp(uint8_t nrc);


signals:
    /**
     * @brief Signals that the ID need to be changed
     * @param id ID to be set
     */
    void setID(uint32_t id);

    /**
     * @brief Signals that Data need to transmitted
     * @param data Data to be transmitted
     */
    void txData(const QByteArray &data);


    /**
     * @brief Signals a Text to be print to GUI console
     */
    void toConsole(const QString &);

public slots:
    /**
     * @brief Slot for received UDS Message to be interpreted
     * @param uds UDS Message
     */
    void rxDataReceiverSlot(const unsigned int id, const QByteArray &ba);

};

#endif /* UDS_LAYER_UDS_H_ */
