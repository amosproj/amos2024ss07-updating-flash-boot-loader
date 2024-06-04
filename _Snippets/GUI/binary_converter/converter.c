// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Leon Wilms <leonwilms.wk@gmail.com>

#include "converter.h"



/*
-----------------------------------------------------------------------------------------------------------
    USED S19 SREC documentation from the wikipedia article here:
    https://en.wikipedia.org/wiki/SREC_(file_format)
-----------------------------------------------------------------------------------------------------------
*/

//TODO clean up
bool convert_s19_to_binary(const char* s19_file) {
    FILE* f_s19 = fopen(s19_file, "r");
    if (!f_s19) {
        printf("Error: Unable to open S19 file.\n");
        return false;
    }

    // Create a binary file with the same name but a .bin extension
    char binary_file[MAX_PATH];
    strncpy(binary_file, s19_file, sizeof(binary_file) - 4);
    strcat(binary_file, ".bin");

    FILE* f_bin = fopen(binary_file, "wb");
    if (!f_bin) {
        printf("Error: Unable to create binary file.\n");
        fclose(f_s19);
        return false;
    }

    //Read one line (This is equal to one entry in S19 files)
    //Maximum length of 1 record is 259 bytes
    char line[259];
    while (fgets(line, sizeof(line), f_s19)) {

        //Each Line has to start with S
        if(line[0] =! 'S'){

            fclose(f_s19);
            fclose(f_bin);

            printf("Corrupt file detected.\n");

            if(remove(f_bin) != 0){

                printf("Error deleting the corrupt bin file.\n");
                return false;
            }
        }

        //S1 is a header for the file with additional ASCII Text information about the file
        //creating another .txt file and writing the information into it.
        if(line[1] == '0'){

            printf("HEADER detected!\n");

            char header_file[MAX_PATH];

            strncpy(header_file, s19_file, sizeof(header_file) - 4);

            strcat(header_file, "_HEADER.txt");

            FILE* f_header = fopen(header_file, "wb");
            if (!f_header) {
                printf("Error: Unable to create binary file.\n");
                fclose(f_header);
                return false;
            }
            char ascii[2];
            char* data_start = line + 4;
            int16_t len;

            sscanf(line + 2, "%2x", &len);

            //len - 1 because we do not want to read the checksum
            char* data_end = line + 4 + (len * 2) - 2;

            while(data_start < data_end){

                sscanf(data_start, "%2x", &ascii);
                fwrite(&ascii, 1, 1, f_header);
                data_start += 2;
            }

            fwrite('\0', 1, 1, f_header);

            fclose(f_header);

            printf("HEADER File successful!\n");
        }

        //extracting S1 data
        if (line[1] == '1') {

            // Insert the hexadecimal number 0x01 to label 16bit adress
            unsigned char hex_number = 0x01;
            fwrite(&hex_number, 1, sizeof(hex_number), f_bin);

            // Extract data bytes and write them to the binary file
            char* data_start = line + 4;
            int16_t len;

            sscanf(line + 2, "%2x", &len);

            //len - 1 because we do not want to read the checksum
            char* data_end = line + 4 + (len * 2) - 2;
            while (data_start < data_end) {
                int byte;
                sscanf(data_start, "%2x", &byte);
                fwrite(&byte, 1, 1, f_bin);
                data_start += 2;
            }

        }
        //extracting S2 data
        else if(line[1] == '2'){

            // Insert the hexadecimal number 0x02 to label 24bit adress
            unsigned char hex_number = 0x02;
            fwrite(&hex_number, 1, sizeof(hex_number), f_bin);

            // Extract data bytes and write them to the binary file
            char* data_start = line + 4;
            int16_t len;

            sscanf(line + 2, "%2x", &len);

            //len - 1 because we do not want to read the checksum
            char* data_end = line + 4 + (len * 2) - 2;
            while (data_start < data_end) {
                int byte;
                sscanf(data_start, "%2x", &byte);
                fwrite(&byte, 1, 1, f_bin);
                data_start += 2;
            }
            // Insert "ZZZ" after processing each line
            unsigned char zz_ascii[3] = {0x5A, 0x5A, 0x5A}; // ASCII for 'ZZZ'
            fwrite(zz_ascii, 1, sizeof(zz_ascii), f_bin);
        }
        //extracting S3 data
        else if(line[1] == '3'){

            // Insert the hexadecimal number 0x03 to label 32bit adress
            unsigned char hex_number = 0x03;
            fwrite(&hex_number, 1, sizeof(hex_number), f_bin);

            // Extract data bytes and write them to the binary file
            char* data_start = line + 4;
            int16_t len;

            sscanf(line + 2, "%2x", &len);

            //len - 1 because we do not want to read the checksum
            char* data_end = line + 4 + (len * 2) - 2;
            while (data_start < data_end) {
                int byte;
                sscanf(data_start, "%2x", &byte);
                fwrite(&byte, 1, 1, f_bin);
                data_start += 2;
            }
            // Insert "ZZZ" after processing each line
            unsigned char zz_ascii[3] = {0x5A, 0x5A, 0x5A}; // ASCII for 'ZZZ'
            fwrite(zz_ascii, 1, sizeof(zz_ascii), f_bin);
        }
        //extracting S5 data
        else if(line[1] == '5'){

            // Insert the hexadecimal number 0x05 to label 16bit count
            unsigned char hex_number = 0x05;
            fwrite(&hex_number, 1, sizeof(hex_number), f_bin);

            // Extract data bytes and write them to the binary file
            char* data_start = line + 4;
            int16_t len;

            sscanf(line + 2, "%2x", &len);

            //len - 1 because we do not want to read the checksum
            char* data_end = line + 4 + (len * 2) - 2;
            while (data_start < data_end) {
                int byte;
                sscanf(data_start, "%2x", &byte);
                fwrite(&byte, 1, 1, f_bin);
                data_start += 2;
            }
            // Insert "ZZZ" after processing each line
            unsigned char zz_ascii[3] = {0x5A, 0x5A, 0x5A}; // ASCII for 'ZZZ'
            fwrite(zz_ascii, 1, sizeof(zz_ascii), f_bin);
        }
        //extracting S6 data
        else if(line[1] == '6'){

            // Insert the hexadecimal number 0x06 to label 24bit count
            unsigned char hex_number = 0x06;
            fwrite(&hex_number, 1, sizeof(hex_number), f_bin);

            // Extract data bytes and write them to the binary file
            char* data_start = line + 4;
            int16_t len;

            sscanf(line + 2, "%2x", &len);

            //len - 1 because we do not want to read the checksum
            char* data_end = line + 4 + (len * 2) - 2;
            while (data_start < data_end) {
                int byte;
                sscanf(data_start, "%2x", &byte);
                fwrite(&byte, 1, 1, f_bin);
                data_start += 2;
            }
            // Insert "ZZZ" after processing each line
            unsigned char zz_ascii[3] = {0x5A, 0x5A, 0x5A}; // ASCII for 'ZZZ'
            fwrite(zz_ascii, 1, sizeof(zz_ascii), f_bin);
        }
        //extracting S7 end of File address
        else if(line[1] == '7'){

            // Insert the hexadecimal number 0x07 to label 32bit adress
            unsigned char hex_number = 0x07;
            fwrite(&hex_number, 1, sizeof(hex_number), f_bin);

            // Extract data bytes and write them to the binary file
            char* data_start = line + 4;
            int16_t len;

            sscanf(line + 2, "%2x", &len);

            //len - 1 because we do not want to read the checksum
            char* data_end = line + 4 + (len * 2) - 2;
            while (data_start < data_end) {
                int byte;
                sscanf(data_start, "%2x", &byte);
                fwrite(&byte, 1, 1, f_bin);
                data_start += 2;
            }
        }
        //extracting S8 end of File address
        else if(line[1] == '8'){

            // Insert the hexadecimal number 0x08 to label 32bit adress
            unsigned char hex_number = 0x08;
            fwrite(&hex_number, 1, sizeof(hex_number), f_bin);

            // Extract data bytes and write them to the binary file
            char* data_start = line + 4;
            int16_t len;

            sscanf(line + 2, "%2x", &len);

            //len - 1 because we do not want to read the checksum
            char* data_end = line + 4 + (len * 2) - 2;
            while (data_start < data_end) {
                int byte;
                sscanf(data_start, "%2x", &byte);
                fwrite(&byte, 1, 1, f_bin);
                data_start += 2;
            }
        }
        //extracting S9 end of File address
        else if(line[1] == '9'){

            // Insert the hexadecimal number 0x09 to label 32bit adress
            unsigned char hex_number = 0x09;
            fwrite(&hex_number, 1, sizeof(hex_number), f_bin);

            // Extract data bytes and write them to the binary file
            char* data_start = line + 4;
            int16_t len;

            sscanf(line + 2, "%2x", &len);

            //len - 1 because we do not want to read the checksum
            char* data_end = line + 4 + (len * 2) - 2;
            while (data_start < data_end) {
                int byte;
                sscanf(data_start, "%2x", &byte);
                fwrite(&byte, 1, 1, f_bin);
                data_start += 2;
            }
        }

    }

    fclose(f_s19);
    fclose(f_bin);
    printf("Binary file saved as: %s\n", binary_file);
    return true;
}
