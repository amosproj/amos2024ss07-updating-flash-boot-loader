#include <stdio.h>
#include <windows.h>
#include <stdbool.h>

/*
-----------------------------------------------------------------------------------------------------------
    REMINDER:
    For this code to work, you will have to link against the "comdlg32" library in the linker settings.
    "GetOpenFileName" will not be found if the linker setup is not correct.
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
        if (line[0] == 'S' && (line[1] == '1' || line[1] == '2' || line[1] == '3')) {
            // Extract data bytes and write them to the binary file
            char* data_start = line + 2;
            char* data_end = line + strlen(line) - 3;
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
