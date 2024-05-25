// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Michael Bauer <mike.bauer@fau.de>, Wiktor Pilarczyk <wiktorpilar99@gmail.com>

//============================================================================
// Name        : UDS.hpp
// Author      : Michael Bauer, Wiktor Pilarczyk
// Version     : 0.3
// Copyright   : MIT
// Description : Qt UDS Layer implementation
//============================================================================
#ifndef UDS_LAYER_UDS_H_
#define UDS_LAYER_UDS_H_

#include <QObject>
#include <QByteArray>

#include "stdint.h"


class UDS : public QObject{
    Q_OBJECT

private:
	uint8_t gui_id;
    uint8_t init;


public:
    UDS();
    UDS(uint8_t gui_id);
	virtual ~UDS();

    void messageInterpreter(unsigned int id, uint8_t *data, uint8_t no_bytes);

	void reqIdentification(); // Sending out broadcast for tester present

	// Specification for Diagnostic and Communication Management
	void diagnosticSessionControl(uint32_t id, uint8_t session);
	void ecuReset(uint32_t id, uint8_t session);
	void securityAccessRequestSEED(uint32_t id);
	void securityAccessVerifyKey(uint32_t id, uint8_t *key, uint8_t key_len);
	void testerPresent(uint32_t id);

	// Specification for Data Transmission
	void readDataByIdentifier(uint32_t id, uint16_t identifier);
	void readMemoryByAddress(uint32_t id, uint32_t address, uint16_t no_bytes); // TODO: Check on Architecture
	void writeDataByIdentifier(uint32_t id, uint16_t identifier, uint8_t* data, uint8_t data_len);

	// Specification for Upload | Download
	void requestDownload(uint32_t id, uint32_t address, uint32_t no_bytes); // TODO: Check on Architecture
	void requestUpload(uint32_t id, uint32_t address, uint32_t no_bytes); // TODO: Check on Architecture
	void transferData(uint32_t id, uint32_t address, uint8_t* data, uint8_t data_len); // TODO: Check on Architecture
	void requestTransferExit(uint32_t id, uint32_t address);

	// Supported Common Response Codes
	void negativeResponse(uint32_t id, uint8_t reg_sid, uint8_t neg_resp_code);


private:
	uint32_t createCommonID(uint32_t base_id, uint8_t gui_id, uint32_t ecu_id);


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

    void resetResponseReceived();

public slots:
    /**
     * @brief Slot for received UDS Message to be interpreted
     * @param uds UDS Message
     */
    void rxDataReceiverSlot(const unsigned int id, const QByteArray &ba);

};

#endif /* UDS_LAYER_UDS_H_ */
