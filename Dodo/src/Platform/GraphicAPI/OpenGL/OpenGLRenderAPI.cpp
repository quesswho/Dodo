#include "pch.h"
#include "OpenGLRenderAPI.h"

#include "Core/Application/Application.h"

#include <backends/imgui_impl_opengl3.h>

namespace Dodo {
	namespace Platform {

		OpenGLRenderAPI::OpenGLRenderAPI(const NativeWindowHandle& handle) 
			: m_Handle(handle), m_ShaderBuilder(0), m_GPUInfo(""), m_VramKbs(0), m_ViewportWidth(0), m_ViewportHeight(0), m_ViewportPosX(0), m_ViewportPosY(0)
		{
			
		}

		OpenGLRenderAPI::~OpenGLRenderAPI() 
		{
			gladLoaderUnloadGL();
		}

		RenderInitError OpenGLRenderAPI::Init(const WindowProperties& winprop)
		{
			m_Context.CreateContextImpl(m_Handle); // Run glad loader
			m_Version = m_Context.LoadGlad();
			std::string versionStr = std::to_string(GLAD_VERSION_MAJOR(m_Version)) + "." + std::to_string(GLAD_VERSION_MINOR(m_Version));
			DD_INFO("OPENGL: {0}", versionStr);
			if (GLAD_VERSION_MAJOR(m_Version) <= 3) {
				return RenderInitError(
					RenderInitStatus::Failed, 
					"OpenGL version < 4.0 is not supported!"
				);
			}
			
			glFrontFace(GL_CCW);
			glEnable(GL_MULTISAMPLE);
			ResizeDefaultViewport(winprop.m_Width, winprop.m_Height);
			m_CullingDefault = winprop.m_Flags & DodoWindowFlags_BACKFACECULL ? 1 : 0;
			Culling(m_CullingDefault);

			glGetIntegerv(0x9048, &m_VramKbs);
			m_GPUInfo = "";
			m_GPUInfo = ((const char*)glGetString(GL_RENDERER));
			m_GPUInfo.append(" VRAM: ").append(StringUtils::KiloByte((size_t)m_VramKbs))
				.append(" : Opengl Version: ").append(versionStr);

			if(Application::s_Application->m_WindowProperties.m_Flags & DodoWindowFlags_IMGUI)
				ImGui_ImplOpenGL3_Init();

			m_ShaderBuilder = new ShaderBuilder();

			return RenderInitError(
					RenderInitStatus::Success, 
					""
				);
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