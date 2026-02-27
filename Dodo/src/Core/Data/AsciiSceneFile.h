#pragma once

#include "Core/Graphics/Scene/Scene.h"
#include "Core/System/DataFile/AsciiDataFile.h"

#include <vector>
#include <string_view>

namespace Dodo {


	class AsciiSceneFile {
	private:
		AsciiDataFile m_File;
	public:
		AsciiSceneFile();
		
		void Write(const char* path, Scene* scene);
		Scene* Read(const char* path);
	private:
		inline void Error(uint line) { DD_WARN("Not able to retrieve entry at line: {}", line); } // kind of silly
	};
}