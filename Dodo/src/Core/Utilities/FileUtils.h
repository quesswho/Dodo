#pragma once

#include "Core/Common.h"
#include "Core/Utilities/Logger.h"

#include <string>
#include <filesystem>
#include <fstream>

namespace Dodo {
	class FileUtils {
	public:
		static std::string ReadFile(const char* path)
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

	};
}