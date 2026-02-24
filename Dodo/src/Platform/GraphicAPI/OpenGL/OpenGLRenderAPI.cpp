#include "pch.h"
#include "OpenGLRenderAPI.h"

#include "Core/Application/Application.h"

#include <backends/imgui_impl_opengl3.h>

namespace Dodo {
	namespace Platform {

		OpenGLRenderAPI::OpenGLRenderAPI() 
			: m_ShaderBuilder(0), m_GPUInfo(""), m_VramKbs(0), m_ViewportWidth(0), m_ViewportHeight(0), m_ViewportPosX(0), m_ViewportPosY(0)
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
					glFrontFace(GL_CCW);
					glEnable(GL_MULTISAMPLE);
					ResizeDefaultViewport(winprop.m_Width, winprop.m_Height);
					m_CullingDefault = winprop.m_Flags & DodoWindowFlags_BACKFACECULL ? 1 : 0;
					Culling(m_CullingDefault);

					glGetIntegerv(0x9048, &m_VramKbs);
					m_GPUInfo = "";
					m_GPUInfo = ((const char*)glGetString(GL_RENDERER));
					m_GPUInfo.append(" VRAM: ").append(StringUtils::KiloByte((size_t)m_VramKbs))
						.append(" : Opengl Version: ").append(std::to_string(GLAD_VERSION_MAJOR(res))).append(".").append(std::to_string(GLAD_VERSION_MINOR(res)));

					if(Application::s_Application->m_WindowProperties.m_Flags & DodoWindowFlags_IMGUI)
						ImGui_ImplOpenGL3_Init();

					m_ShaderBuilder = new ShaderBuilder();
					return 1;
				}
				return -2;
			}
			return -1;
		}

		void OpenGLRenderAPI::DefaultFrameBuffer() const
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, Application::s_Application->m_WindowProperties.m_Width, Application::s_Application->m_WindowProperties.m_Height);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		}

		void OpenGLRenderAPI::Blending(bool blending) const
		{
			if (blending)
			{
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_BLEND);
			}
			else
			{
				glDisable(GL_BLEND);
			}
		}

		void OpenGLRenderAPI::Culling(bool cull, bool backface) {
			if (cull) {
				glEnable(GL_CULL_FACE);
				glCullFace(backface ? GL_BACK : GL_FRONT);
			}
			else {
				glDisable(GL_CULL_FACE);
			}
		}

		void OpenGLRenderAPI::ResizeDefaultViewport(uint width, uint height)
		{
			m_ViewportWidth = width;
			m_ViewportHeight = height;
			glViewport(m_ViewportPosX, m_ViewportPosY, width, height);
		}

		void OpenGLRenderAPI::ResizeDefaultViewport(uint width, uint height, uint posX, uint posY)
		{
			m_ViewportPosX = posX;
			m_ViewportPosY = posY;
			ResizeDefaultViewport(width, height);
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