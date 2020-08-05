#pragma once
#include <vector>
#include <string_view>
#include "Core/Graphics/Scene/Scene.h"

namespace Dodo {


	class AsciiSceneFile {
	public:
		AsciiSceneFile();
		
		void Write(const char* path, Scene* scene); // TODO: add flag to pack model data in file
	};
}