#pragma once

#include "Core/Common.h"

namespace Dodo {

	enum class TextureFilter : uint
	{
		FILTER_MIN_MAG_LINEAR_MIP_NEAREST,
		FILTER_MIN_MAG_MIP_LINEAR,
		FILTER_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST,
		FILTER_MIN_NEAREST_MAG_MIP_LINEAR,

		FILTER_MIN_MAG_MIP_NEAREST,
		FILTER_MIN_MAG_NEAREST_MIP_LINEAR,
		FILTER_MIN_LINEAR_MAG_MIP_NEAREST,
		FILTER_MIN_LINEAR_MAG_MIP_LINEAR
	};

	enum class TextureWrapMode
	{
		WRAP_REPEAT,
		WRAP_CLAMP_TO_BORDER,
		WRAP_CLAMP_TO_EDGE,
		WRAP_MIRRORED_REPEAT
	};

	struct TextureProp {
		TextureFilter m_Filter;
		TextureWrapMode m_WrapU;
		TextureWrapMode m_WrapV;
	};

	namespace Platform {

		class OpenGLTexture {
		private:
			uint m_TextureID;
		public:
			OpenGLTexture(const char* path, uint index, const TextureProp& prop);
			~OpenGLTexture();

			void Bind() const;
		public:
			uint m_Index;
		private:
			int m_Width, m_Height;
		};
	}
}