#pragma once
#include <filesystem>

class FileDialog {
public:
    static std::filesystem::path OpenFile(const char* title, const char* filterPatterns = nullptr);

    static std::filesystem::path SaveFile(const char* title, const char* defaultPath = nullptr);
};