#pragma once

#include "Core/Graphics/Material/TextureProperties.h"

namespace Dodo {

	namespace Platform {

		class OpenGLTexture {
		private:
			uint m_TextureID;
		public:
			OpenGLTexture(const char* path, uint index = 0, const TextureProperties& prop = TextureProperties());
			~OpenGLTexture();

			void Bind() const;
		public:
			uint m_Index;
		private:
			int m_Width, m_Height;
		};
	}
}