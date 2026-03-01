#pragma once

#include <Core/Common.h>

#include "Core/Utilities/Logger.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <vector>

namespace Dodo {
    class FileUtils {
      public:
        static std::string ReadTextFile(const char *path)
        {
            std::ifstream file(path);
            if (file.good())
            {
                std::string result;
                const uint64 size = std::filesystem::file_size(path);
                result.resize(size);
                file.read(result.data(), size);

                return result;
            }
            DD_ERR("Failed to read file: {}", path);
            return std::string("-1");
        }

        static void WriteTextFile(const char *path, std::vector<std::string> data)
        {
            std::ofstream file(path);
            for (std::string str : data)
                file << str << "\n";
            file.close();
        }

        static bool FileExists(const char *path)
        {
            struct stat buffer;
            return (stat(path, &buffer) == 0);
        }

        static void RemoveDoubleBackslash(std::string &str)
        {
            auto it = std::find(str.begin(), str.end(), '\\');
            while (it != str.end())
            {
                str.replace(it, it + 1, "/");

                it = std::find(it + 2, str.end(), '\\');
            }
        }
    };
} // namespace Dodo