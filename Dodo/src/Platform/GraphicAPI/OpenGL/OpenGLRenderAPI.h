#pragma once

#include "Core/Application/Window.h"

#include <glad/gl.h>

namespace Dodo {

	namespace Platform {

		class OpenGLRenderAPI {
		public:
			OpenGLRenderAPI();
			~OpenGLRenderAPI();
			int Init(const WindowProperties& winprop);

			inline void Begin() const { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }
			inline void End() const {}
			void ImGuiNewFrame() const;
			void ImGuiEndFrame() const;

			inline void ClearColor(float r, float g, float b) const { glClearColor(r, g, b, 1.0f); }
			inline void Viewport(uint width, uint height) const { glViewport(0, 0, (GLsizei)width, (GLsizei)height); }
			inline void DrawIndices(uint count) const { glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0); }
			void DefaultFrameBuffer() const;
			void ResizeDefaultViewport(uint width, uint height);
			void ResizeDefaultViewport(uint width, uint height, uint posX, uint posY);

			inline void DepthTest(bool depthtest) const { depthtest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST); }
			inline void StencilTest(bool stenciltest) const { stenciltest ? glEnable(GL_STENCIL_TEST) : glDisable(GL_STENCIL_TEST); }
			void Blending(bool blending) const 
			{
				if (blending)
				{
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					glEnable(GL_BLEND);
				}
				else
					glDisable(GL_BLEND);
			}

			inline const char* GetAPIName() const { return "OpenGL"; }
			int CurrentVRamUsage() const
			{
				int availKb;
				glGetIntegerv(0x9049, &availKb); // Current available

				return m_VramKbs - availKb;
			}

			std::string m_GPUInfo;
			int m_VramKbs;

			uint m_ViewportWidth, m_ViewportHeight, m_ViewportPosX, m_ViewportPosY;
		private:
		};
	}
}