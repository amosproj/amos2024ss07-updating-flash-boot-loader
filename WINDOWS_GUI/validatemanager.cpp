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
#include "CCRC32.h"
#include <string.h>

ValidateManager::ValidateManager() {

    data.clear();
    checksums.clear();

}


QMap<uint32_t, QByteArray> ValidateManager::validateFile(QByteArray data)
{
    QList<QByteArray> lines = data.split('\n');
    int current_index = 0;
    int result_index = 0;

    int count_record = -1;
    int count_lines = 0;
    int nlines = lines.size();

    bool file_validity = true;
    bool file_header = false;

    QByteArray new_line;
    QMap<uint32_t, QByteArray> result;

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

            file_validity = false;
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

            emit updateLabel(ValidateManager::HEADER, "File header:  " + asciiString);


        }
        // preprocess data for flashing and count data records for validation
        else if(record_type == '1' or record_type == '2' or record_type == '3'){

            new_line = extractData(line, record_type);

            result.insert(result_index, new_line);

            result_index += 1;
            count_lines += 1;
        }
        // preprocess data for flashing
        else if(record_type == '7' or record_type == '8' or record_type == '9'){

            new_line = extractData(line, record_type);

            result.insert(result_index, new_line);

            result_index += 1;
        }
        // optional entry, can be used to validate file
        else if(record_type == '5' or record_type == '6'){

            line = line.left(line.size() - 2);
            line = line.right(line.size() - 2);

            if(count_record != -1){

                file_validity = false;
            }

            count_record = line.toInt(NULL, 16);
        }
        // S4 is reserved and should not be used & all other inputs are invalid s19 inputs
        else {

            qDebug() << "There was an error with the selected file! Record type: " << record_type << line;
            emit errorPrint((&"ERROR: There was an error with the selected file! Record Type: " [ record_type]));
            emit errorPrint("ERROR: Test");
        }

        current_index += 1;
    }

    if(file_header != true){

        emit updateLabel(ValidateManager::HEADER, "File header:  N/A");
    }

    // Check if count record is present and is correctly set
    if(count_record != count_lines and count_record != -1){

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


    return result;
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
        uint32_t hexValue = hexPair.toInt(&ok, 16); // Convert hexPair to ushort (16-bit) integer

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

        qDebug() << "There was an error! with current line!";
        emit errorPrint("There was an error with current line!\n");
    }

    return trimmed_line;
}

QMap<uint32_t, uint32_t> ValidateManager::calculateFileChecksums(QMap<uint32_t, QByteArray> data)
{   /*
    int numEntries = data.size();
    const char *dataCharP;
    QString dataString = "";

    for (int index = 0; index < numEntries; index++) {
        QByteArray nextLine = data.value(index);
        nextLine.remove(0, 8); //removes address from entry
        QString newData = QString(nextLine);
        dataString += newData;
    }

    QByteArray conversion = dataString.toLocal8Bit();

    dataCharP = conversion.data();

    CCRC32 crc;
	crc.Initialize();
    
    return crc.FullCRC((const unsigned char *)dataCharP, strlen(dataCharP));*/
    
    QMap<uint32_t, uint32_t> result;

    for (auto [key, value] : data.asKeyValueRange()) {
        CCRC32 crc;
        crc.Initialize();

        const char *nextLine = value.data();

        uint32_t checksum = (uint32_t) crc.FullCRC((const unsigned char *) nextLine, strlen(nextLine));
        result.insert(key, checksum);
    }

    return result;
}
