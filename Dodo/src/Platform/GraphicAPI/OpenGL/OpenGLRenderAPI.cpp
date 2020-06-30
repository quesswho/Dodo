#include "pch.h"
#include "OpenGLRenderAPI.h"
#include "Core/Application/Application.h"

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
					glFrontFace(GL_CCW);
					glGetIntegerv(0x9048, &m_VramKbs);
					m_GPUInfo = "";
					m_GPUInfo = ((const char*)glGetString(GL_RENDERER));
					m_GPUInfo.append(" VRAM: ").append(StringUtils::KiloByte((size_t)m_VramKbs))
						.append(" : Opengl Version: ").append(std::to_string(GLAD_VERSION_MAJOR(res))).append(".").append(std::to_string(GLAD_VERSION_MINOR(res)));

					if(Application::s_Application->m_WindowProperties.m_ImGUI)
						ImGui_ImplOpenGL3_Init();
					return 1;
				}
				return -2;
			}
			return -1;
		}

		void OpenGLRenderAPI::ImGuiNewFrame() const
		{
			ImGui_ImplOpenGL3_NewFrame();
		}

		void OpenGLRenderAPI::ImGuiEndFrame() const
		{
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
	}
}