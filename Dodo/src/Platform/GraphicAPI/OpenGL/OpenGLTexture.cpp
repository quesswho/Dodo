#include "pch.h"
#include "OpenGLTexture.h"

namespace Dodo {

	namespace Platform {

		OpenGLTexture::OpenGLTexture(const char* path, uint index, const TextureProp& prop)
			: m_Index(index)
		{

		}

		OpenGLTexture::~OpenGLTexture()
		{}

		void OpenGLTexture::Bind() const
		{

		}
	}
}