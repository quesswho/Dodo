#pragma once

#include "Core/Common.h"

namespace Dodo {

	typedef enum TextureFilter {
		DODO_FILTER_MIN_MAG_LINEAR_MIP_NEAREST,
		DODO_FILTER_MIN_MAG_MIP_LINEAR,
		DODO_FILTER_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST,
		DODO_FILTER_MIN_NEAREST_MAG_MIP_LINEAR,

		DODO_FILTER_MIN_MAG_MIP_NEAREST,
		DODO_FILTER_MIN_MAG_NEAREST_MIP_LINEAR,
		DODO_FILTER_MIN_LINEAR_MAG_MIP_NEAREST,
		DODO_FILTER_MIN_LINEAR_MAG_MIP_LINEAR
	};

	struct TextureProp {
		TextureFilter m_Filter;
	};

	namespace Platform {

		class OpenGLTexture {
		private:
			uint m_TextureID;
			uint m_Index;
		public:
			OpenGLTexture(const char* path, uint index, const TextureProp& prop);
			~OpenGLTexture();

			void Bind() const;
		};
	}
}