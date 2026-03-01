#include "FileDialog.h"
#include <tinyfiledialogs.h>

std::string FileDialog::OpenFile(const char* title, const char* filters) {
    const char* path = tinyfd_openFileDialog(title, "", 0, nullptr, filters, 0);
    if (!path) return std::string();
    return std::string(path);
}

std::string FileDialog::SaveFile(const char* title, const char* defaultPath) {
    const char* path = tinyfd_saveFileDialog(title, defaultPath, 0, nullptr, nullptr);
    if (!path) return std::string();
    return std::string(path);
}