#include "pch.h"
#include "OpenGLRenderAPI.h"
#include <glad/gl.h>
#include <imgui_impl_opengl3.h>

namespace Dodo {
	namespace Platform {

		OpenGLRenderAPI::OpenGLRenderAPI() 
			: m_GPUInfo(""), m_VramKbs(0)
		{}

		OpenGLRenderAPI::~OpenGLRenderAPI() 
		{
			gladLoaderUnloadGL();
		}

		int OpenGLRenderAPI::Init(const WindowProperties& winprop)
		{
			int res = gladLoaderLoadGL();
			if (res)
			{
				if (GLAD_VERSION_MAJOR(res) > 3)
				{
					Viewport(winprop.m_Width, winprop.m_Height);

					glGetIntegerv(0x9048, &m_VramKbs);
					m_GPUInfo = "";
					m_GPUInfo = ((const char*)glGetString(GL_RENDERER));
					m_GPUInfo.append(" VRAM: ").append(StringUtils::KiloByte((size_t)m_VramKbs))
						.append(" : Opengl Version: ").append(std::to_string(GLAD_VERSION_MAJOR(res))).append(".").append(std::to_string(GLAD_VERSION_MINOR(res)));

					ImGui_ImplOpenGL3_Init();
					return 1;
				}
				return -2;
			}
			return -1;
		}

		void OpenGLRenderAPI::Begin() const
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		}

		void OpenGLRenderAPI::ClearColor(float r, float g, float b) const
		{
			glClearColor(r, g, b, 1.0f);
		}

		void OpenGLRenderAPI::Viewport(uint width, uint height) const
		{
			glViewport(0, 0, (GLsizei)width, (GLsizei)height);
		}

		const char* OpenGLRenderAPI::GetAPIName() const
		{
			return "OpenGL";
		}

		int OpenGLRenderAPI::CurrentVRamUsage() const
		{
			int availKb;
			glGetIntegerv(0x9049, &availKb); // Current available

			return m_VramKbs - availKb;
		}
	}
}