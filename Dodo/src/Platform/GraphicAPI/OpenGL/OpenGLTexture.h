#pragma once

#include "Core/Graphics/Material/TextureProperties.h"

namespace Dodo {

	namespace Platform {

		class OpenGLTexture {
		private:
			uint m_TextureID;
		public:
			OpenGLTexture(const char* path, uint index = 0, const TextureSettings& settings = TextureSettings());
			OpenGLTexture(uchar* data, TextureProperties prop, uint index = 0, const TextureSettings& settings = TextureSettings());
			~OpenGLTexture();

			void Bind() const;
		private:
			void Init(uchar* data, const TextureSettings& settings);
		public:
			uint m_Index;
			const TextureProperties& GetTextureProperties() const { return m_TextureProperties; }
		private:
			TextureProperties m_TextureProperties;
		};
	}
}