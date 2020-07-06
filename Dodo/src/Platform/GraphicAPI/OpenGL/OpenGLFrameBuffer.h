#pragma once
#include "Core/Common.h"

#include <glad/gl.h>

namespace Dodo {

	enum class FrameBufferType
	{
		FRAMEBUFFER_COLOR_DEPTH_STENCIL,
		FRAMEBUFFER_DEPTH
	};

	struct FrameBufferProperties {
		FrameBufferProperties() 
			: m_Width(0), m_Height(0), m_FrameBufferType(FrameBufferType::FRAMEBUFFER_COLOR_DEPTH_STENCIL)
		{}
		uint m_Width, m_Height;
		FrameBufferType m_FrameBufferType;

	};

	namespace Platform {

		class OpenGLFrameBuffer {
		private:
			uint m_FrameBufferID, m_TextureID, m_RenderBuffer;
			FrameBufferProperties m_FrameBufferProperties;
		public:
			OpenGLFrameBuffer(const FrameBufferProperties& framebufferprop);
			~OpenGLFrameBuffer();

			inline void Bind() const { 
				glViewport(0, 0, m_FrameBufferProperties.m_Width, m_FrameBufferProperties.m_Height);
				glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID); 
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			}

			inline void BindTexture(uint index = 0) const { 
				glActiveTexture(GL_TEXTURE0 + index);
				glBindTexture(GL_TEXTURE_2D, m_TextureID); 
			}

			void Resize(uint width, uint height);
			inline uint GetTextureHandle() { return m_TextureID; }
		private:
			void Create();
		};
	}
}