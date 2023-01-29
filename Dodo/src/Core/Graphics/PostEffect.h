#pragma once

#include "Core/Common.h"
#include "Core/Application/Application.h"

#include "Core/Graphics/FrameBuffer.h"
#include "Core/Graphics/Material/Shader.h"
#include "Core/Graphics/Buffer.h"

namespace Dodo { 
	class PostEffect {
		private:
			VertexBuffer* m_Vertexbuffer;
			FrameBuffer* m_Framebuffer;
			Shader* m_Shader;
		public:
			PostEffect(const FrameBufferProperties& framebufferprop, const char* path);
			~PostEffect();

			inline void Bind() const {
				m_Framebuffer->Bind();
			}

			template<typename T>
			void SetUniformValue(const char* location, const T val) {
				m_Shader->Bind();
				m_Shader->SetUniformValue(location, val);
			}

			inline void Draw() const {
				Application::s_Application->m_RenderAPI->DefaultFrameBuffer();
				m_Shader->Bind();
				m_Vertexbuffer->Bind();
				m_Framebuffer->BindTexture();
				Application::s_Application->m_RenderAPI->DrawArray(6);
			}

			void Resize(uint width, uint height) { m_Framebuffer->Resize(width, height); }
		private:
			void Create();
	};
}

