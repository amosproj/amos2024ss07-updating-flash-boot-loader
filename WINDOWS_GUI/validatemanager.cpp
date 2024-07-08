// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Leon Wilms <leonwilms.wk@gmail.com>

//============================================================================
// Name        : validatemanager.cpp
// Author      : Leon Wilms
// Version     : 0.1
// Copyright   : MIT
// Description : Validation Manager to validate selected files
//============================================================================

#include "validatemanager.h"
#include <string.h>

#include <QDebug>
#include <QPointer>
#include <QApplication>


//============================================================================
// Constructor
//============================================================================

ValidateManager::ValidateManager() {

    data.clear();
    core_addr.clear();
}

ValidateManager::~ValidateManager(){
    data.clear();
    core_addr.clear();
}

//============================================================================
// Public Method
//============================================================================



void ValidateManager::validateFileAsync(QByteArray data){

    // For Null pointer safety
    QPointer<ValidateManager> self = this;

    // Change cursor to loading state
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Create thread for validation
    QThread* thread = QThread::create([self, data]() {

        if (!self) {

            return;
        }

        QMap<uint32_t, QByteArray> result;
        {
            QMutexLocker locker(&self->dataMutex);
            result = self->validateFile(data);
        }
        emit self->validationDone(result);
    });
    thread->start();
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    // Use a queued connection to restore the cursor in the main thread
    connect(thread, &QThread::finished, []() {
        QMetaObject::invokeMethod(qApp, []() {
                QApplication::restoreOverrideCursor();
            }, Qt::QueuedConnection);
    });
}

bool ValidateManager::checkBlockAddressRange(const QMap<uint32_t, QByteArray> blocks){

    for (QMap<uint32_t, QByteArray>::const_iterator iterator = blocks.constBegin(); iterator != blocks.constEnd(); ++iterator) {

        uint32_t addr = iterator.key();

        QByteArray block = blocks[addr];
        uint32_t data_len = block.size();

        if(!addrInRange(addr, data_len)){

            emit infoPrint("INFO: File not Valid! Data with len "+QString("0x%1").arg(data_len, 2, 16, QLatin1Char( '0' ))+" would be written into reserved memory. -> Address: "+ QString("0x%1").arg(addr, 2, 16, QLatin1Char( '0' ))+"\n");
            return false;
        }

    }

    return true;
}

//============================================================================
// Private Method
//============================================================================

QMap<uint32_t, QByteArray> ValidateManager::validateFile(QByteArray data)
{
    QList<QByteArray> lines = data.split('\n');
    uint32_t block_address_end = 0;

    int current_index = 0;

    int count_record = -1;
    int jump_record = -1;

    int count_lines = 0;
    int nlines = lines.size();

    bool file_validity = true;
    bool file_header = false;

    QByteArray new_line;
    QByteArray result;
    QByteArray line_buffer;

    QMap<uint32_t, QByteArray> merged_blocks;
    QMap<uint32_t, QByteArray> block_result;

    // Print each line
    for (QByteArray& line : lines) {

        if(line.size() == 0){

            break;
        }

        // remove '\r' at the end of each line
        if(current_index != (nlines - 1)){

            line = line.left(line.size() - 1 );
        }

        char record_type = line.at(1);              // get record type
        line = line.mid(2);                         // Trim Record type for preprocessing

        // Check validity of one line at a time
        if(!validateLine(line))
        {
            emit infoPrint("INFO: File not valid! Checksum of a line did not match the expected value.\n");
            file_validity = false;
            break;
        }

        // extract header information
        if(record_type == '0'){

            QByteArray header;

            line = line.left(line.size() - 2);
            line = line.right(line.size() - 2);

            file_header = true;


            for (int i = 0; i < line.size(); i += 2)
            {
                // Convert each pair of hex characters to a byte
                QString hexPair = line.mid(i, 2);

                char asciiChar = hexPair.toUShort(NULL, 16); // Convert hexPair to integer

                if(asciiChar != NULL){

                    header.append(asciiChar);
                }

            }

            // Convert QByteArray to QString (assuming it's ASCII)
            QString asciiString = QString::fromLatin1(header);

            emit updateLabel(ValidateManager::HEADER, "File version: " + asciiString);


        }
        // preprocess data for flashing and count data records for validation
        else if(record_type == '1' or record_type == '2' or record_type == '3'){

            new_line = extractData(line, record_type);

            uint32_t data_len = (new_line.size() - 8);

            QString address_string = new_line.left(8);
            uint32_t address_start = address_string.toUInt(NULL, 16);

            if(line_buffer.isEmpty()){

                line_buffer.append(new_line.right(data_len + 8));
                count_lines += 1;

            }
            else if(block_address_end == address_start){

                line_buffer.append(new_line.right(data_len));
                count_lines += 1;
            }
            else{
                merged_blocks.insert(getAddr(line_buffer.left(8).toUInt(NULL, 16)), getData(line_buffer.right(line_buffer.size()-8)));

                line_buffer.clear();

                line_buffer.append(new_line.right(data_len + 8));
                count_lines += 1;
            }

            // calculate new ending of this block
            block_address_end = address_start + (data_len / 2);
        }
        // preprocess data for flashing
        else if(record_type == '7' or record_type == '8' or record_type == '9'){

            // TODO: How to handle jump addresses?
            //currently deprecated.
            emit infoPrint("INFO: Jump addresses not supported!");
            continue;
            if(jump_record != -1){

                emit infoPrint("INFO: File not Valid! Too many termination records.\n");
                file_validity = false;
                break;
            }

            if(current_index == (nlines - 1)){
                merged_blocks.insert(getAddr(line_buffer.left(8).toUInt(NULL, 16)), getData(line_buffer.right(line_buffer.size()-8)));

                line_buffer.clear();
            }

            new_line = extractData(line, record_type);


            //merged_blocks.insert(block_index, new_line);
        }
        // optional entry, can be used to validate file
        else if(record_type == '5' or record_type == '6'){

            if(count_record != -1){

                emit infoPrint("INFO: File not Valid! Too many count records.\n");
                file_validity = false;
                break;
            }

            line = line.left(line.size() - 2);
            line = line.right(line.size() - 2);

            count_record = line.toInt(NULL, 16);
        }
        // S4 is reserved and should not be used & all other inputs are invalid s19 inputs
        else {

            qDebug() << "There was an error with the selected file! Record type: " << record_type << line;
            emit errorPrint(("ERROR: There was an error with the selected file! Record Type: " + QString::number(record_type) + "\n"));
            emit errorPrint("ERROR: Test");
            break;
        }

        current_index += 1;
    }

    if(!line_buffer.isEmpty()){
        merged_blocks.insert(getAddr(line_buffer.left(8).toUInt(NULL, 16)), getData(line_buffer.right(line_buffer.size()-8)));
        line_buffer.clear();
    }
    block_result = combineSortedQMap(merged_blocks);

    //TODO: remove

    if(file_validity){

        file_validity = checkBlockAddressRange(block_result);
    }


    if(!file_header){

        emit updateLabel(ValidateManager::HEADER, "File version: N/A");
    }

    // Check if count record is present and is correctly set
    if(file_validity && count_record != count_lines && count_record != -1){

        emit infoPrint("INFO: File not valid! Count record does not match the number of data records.\n");
        file_validity = false;
    }

    // Show validity of file in UI
    if(file_validity)
    {

        emit updateLabel(ValidateManager::VALID, "File validity:  Valid");

    }
    else {

        emit updateLabel(ValidateManager::VALID, "File validity:  Not Valid");
    }
    return block_result;
}

bool ValidateManager::validateLine(QByteArray line)
{
    int sum = 0;
    int checksum = line.right(2).toInt(NULL, 16); // Extracts the last two characters

    // Remove the checksum from the original line for further processing
    QByteArray trimmedLine = line.left(line.size() - 2);

    // Process the line in pairs of hexadecimal values
    for (int i = 0; i < trimmedLine.size(); i += 2)
    {
        // Extract each pair of characters
        QByteArray hexPair = trimmedLine.mid(i, 2);

        // Convert the hexPair to a hexadecimal value
        bool ok;
        int hexValue = hexPair.toInt(&ok, 16); // Convert hexPair to ushort (16-bit) integer

        if (!ok)
        {
            // Handle conversion error
            qDebug() << "Error converting hexPair:" << hexPair;
            emit errorPrint("ERROR: Error converting hexPair:" + hexPair + "\n");

            return false;
        }
        else
        {

            sum += hexValue;
        }

    }

    if(checksum == (0xFF - (sum & 0xFF))){

        return true;
    }

    return false;
}

QByteArray ValidateManager::extractData(QByteArray line, char record_type)
{
    // Trim Count
    QByteArray trimmed_line = line.left(line.size() - 2);
    // Trim Checksum
    trimmed_line = trimmed_line.right(trimmed_line.size() - 2);

    // Pad address
    if(record_type == '1' or record_type == '9'){

        trimmed_line = trimmed_line.rightJustified(trimmed_line.size() + 4, '0');
    }
    else if(record_type == '2' or record_type == '8'){

        trimmed_line = trimmed_line.rightJustified(trimmed_line.size() + 2, '0');
    }
    else if(record_type == '3' or record_type == '7'){

        // There is nothing to pad, dont do anything
    }
    // Did we encounter a S-record that is not supposed to be here?
    else{

        qDebug() << "ERROR: There was an error! with current line!";
        emit errorPrint("ERROR: There was an error with current line!\n");
    }

    return trimmed_line;
}


QMap<uint32_t, QByteArray> ValidateManager::combineSortedQMap(const QMap<uint32_t, QByteArray> blocks){

    QMap<uint32_t, QByteArray> merged_blocks;

    if (blocks.isEmpty()) {
        return merged_blocks;
    }

    QByteArray line_buffer;

    uint32_t new_start_addr = blocks.firstKey();
    uint32_t addr_end = 0;

    for (QMap<uint32_t, QByteArray>::const_iterator iterator = blocks.constBegin(); iterator != blocks.constEnd(); ++iterator){

        uint32_t addr_start = iterator.key();

        if(addr_start == addr_end){

            line_buffer.append(blocks[addr_start]);
        }
        else{

            merged_blocks.insert(new_start_addr, line_buffer);
            line_buffer.clear();

            new_start_addr = addr_start;
            line_buffer.append(blocks[addr_start]);
        }

        addr_end = addr_start + blocks[addr_start].size();
    }

    merged_blocks.insert(new_start_addr, line_buffer);

    return merged_blocks;
}

bool ValidateManager::addrInCoreRange(uint32_t addr, uint32_t data_len,  uint16_t core, bool* supported){

    QString core_start_add_string = core_addr[core]["start"];
    QString core_end_add_string = core_addr[core]["end"];

    if(core_start_add_string == "Not yet supported" || core_end_add_string == "Not yet supported" || core_start_add_string == "" || core_end_add_string == ""){

        *supported = false;
        //qDebug() << "INFO: Address Validation not supported for core" << core << "!";

        return true;
    }

    uint32_t core_start_add = core_start_add_string.toUInt(NULL, 16);
    uint32_t core_end_add = core_end_add_string.toUInt(NULL, 16);

    if((core_start_add > 0 && core_end_add > 0) && (addr >= core_start_add && addr < core_end_add)){

        if(addr + (data_len-1) <= core_end_add){

            return true;
        }
    }

    return false;
}


bool ValidateManager::addrInRange(uint32_t address, uint32_t data_len){

    bool supported = true;

    if( addrInCoreRange(address, data_len, 0, &supported) ||
        addrInCoreRange(address, data_len, 1, &supported) ||
        addrInCoreRange(address, data_len, 2, &supported) ||
        addrInCoreRange(address, data_len, 3, &supported) ||
        addrInCoreRange(address, data_len, 4, &supported))
    {

        return true;
    }

    if(!supported){

        emit infoPrint("INFO: Address validation not supported for one or more cores! Please check debugging output! \n");

    }

    return false;
}

//============================================================================
// Private Helper Method
//============================================================================

QByteArray ValidateManager::getData(QByteArray tempData) {
    QByteArray transformedData;
    for (int i = 0; i < tempData.size(); i += 2) {
        QString hexByte = tempData.mid(i, 2);
        transformedData.append((uint8_t)(0xFF & hexByte.toUInt(NULL, 16)));
    }
    return transformedData;
}

uint32_t ValidateManager::getAddr(uint32_t addr){
    //TODO: Change after decision about how to handle addresses: A00.... vs 800...
    if((addr & 0x80000000) > 0)
        addr |= 0xA0000000;

    return addr;
}
