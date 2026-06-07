#include "FileDialog.h"
#include <tinyfiledialogs.h>

// TODO: Save this to file
static std::string s_LastOpenDirectory = std::filesystem::current_path().string() + "/";
static std::string s_LastSavedDirectory = std::filesystem::current_path().string() + "/";

std::filesystem::path FileDialog::OpenFile(const char* title, const char* filters)
{
    const char* path = tinyfd_openFileDialog(title, s_LastOpenDirectory.c_str(), 0, nullptr, filters, 0);
    if (!path) return std::filesystem::path();
    std::filesystem::path result(path);
    s_LastOpenDirectory = result.parent_path().string() + "/";
    return result;
}

std::filesystem::path FileDialog::SaveFile(const char* title, const char* filters)
{
    const char* path = tinyfd_saveFileDialog(title, s_LastSavedDirectory.c_str(), 0, nullptr, filters);
    if (!path) return std::filesystem::path();
    std::filesystem::path result(path);
    s_LastSavedDirectory = result.parent_path().string() + "/";
    return result;
}

std::filesystem::path FileDialog::SelectDirectory(const char* title)
{
    const char* path = tinyfd_selectFolderDialog(title, s_LastOpenDirectory.c_str());
    if (!path) return std::filesystem::path();
    std::filesystem::path result(path);
    s_LastOpenDirectory = result.string() + "/";
    return result;
}