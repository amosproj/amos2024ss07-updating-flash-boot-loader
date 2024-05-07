#include "converter.h"

/*
-----------------------------------------------------------------------------------------------------------
    ATTENTION:
    For this code to work, you will have to link against the "comdlg32" library in the linker settings.
    "GetOpenFileName" will not be found if the linker setup is not correct.
-----------------------------------------------------------------------------------------------------------
*/

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
