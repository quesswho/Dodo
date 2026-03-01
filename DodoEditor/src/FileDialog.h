#pragma once
#include <optional>
#include <string>

class FileDialog {
public:
    static std::string OpenFile(const char* title, const char* filterPatterns = nullptr);

    static std::string SaveFile(const char* title, const char* defaultPath = nullptr);
};