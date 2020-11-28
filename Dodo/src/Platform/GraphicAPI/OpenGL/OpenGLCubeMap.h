#pragma once

#include <vector>
#include <string>
#include "Core/Graphics/Material/TextureProperties.h"

namespace Dodo {

	namespace Platform {

		class OpenGLCubeMapTexture {
		private:
			uint m_TextureID;
		public:
			OpenGLCubeMapTexture(std::vector<std::string> paths, uint index = 0, const TextureSettings& prop = TextureSettings());
			~OpenGLCubeMapTexture();

			void Bind() const;
		public:
			uint m_Index;
		};
	}
}