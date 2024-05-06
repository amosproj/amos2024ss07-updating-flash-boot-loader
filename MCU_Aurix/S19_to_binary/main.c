#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

/*
-----------------------------------------------------------------------------------------------------------
    REMINDER:
    For this code to work, you will have to link against the "comdlg32" library in the linker settings.
    "GetOpenFileName" will not be found if the linker setup is not correct.
-----------------------------------------------------------------------------------------------------------

    USED S19 SREC documentation from the wikipedia article here:
    https://en.wikipedia.org/wiki/SREC_(file_format)

-----------------------------------------------------------------------------------------------------------
*/



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

    char line[256];
    while (fgets(line, sizeof(line), f_s19)) {

        if(line[0] =! 'S'){

            fclose(f_s19);
            fclose(f_bin);

            printf("Corrupt file detected.\n");

            if(remove(f_bin) != 0){

                printf("Error deleting the corrupt bin file.\n");
                return 1;
            }
        }

        if(line[1] == '0'){

            printf("HEADER detected!\n");

            char header_file[MAX_PATH];



            strncpy(header_file, s19_file, sizeof(header_file) - 4);

            strcat(header_file, "_HEADER.txt");

            printf("HEADER_name: %s \n", header_file);

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

                printf("data_start in loop: %d\n", data_start);
                printf("data_end in loop: %d\n\n", data_end);

                //printf("I: %d\n", len-data_start);

                sscanf(data_start, "%2x", &ascii);

                printf("ascii char: %s\n", ascii);

                fwrite(&ascii, 1, 1, f_header);
                data_start += 2;
            }

            fwrite('\0', 1, 1, f_header);

            fclose(f_header);

            printf("HEADER File successful!\n");
        }


        if (line[1] == '1' || line[1] == '2' || line[1] == '3') {
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

int main() {
    OPENFILENAME ofn;
    char szFile[MAX_PATH];
    HWND hwnd = NULL;

    // Initialize OPENFILENAME structure
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "S19 Files (*.s19)\0*.s19\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the file dialog
    if (GetOpenFileName(&ofn) == TRUE) {
        // User selected an S19 file
        printf("Selected S19 file: %s\n", ofn.lpstrFile);
        if (convert_s19_to_binary(ofn.lpstrFile)) {
            printf("Conversion successful.\n");
        } else {
            printf("Conversion failed.\n");
        }
    } else {
        printf("No file selected.\n");
    }

    return 0;
}
