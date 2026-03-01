#include "FileDialog.h"
#include <tinyfiledialogs.h>

std::filesystem::path FileDialog::OpenFile(const char *title, const char *filters)
{
    const char *path = tinyfd_openFileDialog(title, "", 0, nullptr, filters, 0);
    if (!path)
        return std::filesystem::path();
    return std::filesystem::path(path);
}

std::filesystem::path FileDialog::SaveFile(const char *title, const char *filters)
{
    const char *path = tinyfd_saveFileDialog(title, "", 0, nullptr, filters);
    if (!path)
        return std::filesystem::path();
    return std::filesystem::path(path);
}