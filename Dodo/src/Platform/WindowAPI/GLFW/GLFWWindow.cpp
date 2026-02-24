#include "pch.h"
#include "GLFWWindow.h"

#include "Core/System/FileUtils.h"

#include "Core/Application/Application.h"

#include <GLFW/glfw3.h>

namespace Dodo {

	namespace Platform {

		GLFWWindow::GLFWWindow(const WindowProperties& winProp)
			: m_WindowProperties(winProp), m_Focused(true)
		{
			Init();
		}

		GLFWWindow::~GLFWWindow()
		{
			glfwTerminate();
		}

		void GLFWWindow::Init()
		{
			// Fail if we can not initialize
			if (!glfwInit())
			{
				DD_FATAL("Could not initialize GLFW!");
				return;
			}

			// Set glfw error callback. This maps to the logger
			glfwSetErrorCallback(ErrorCallback);
			ConfigureMonitor();
			GLFWwindow* window = glfwCreateWindow(m_WindowProperties.m_Width, 
				m_WindowProperties.m_Height, 
				m_WindowProperties.m_Title, 
				NULL, NULL);
				if (!window)
			{
				DD_FATAL("Could not create GLFW window!");
				return;
			}
			CreateDeviceContext();
		}

		void GLFWWindow::Update() const
		{}

		void GLFWWindow::SetTitle(const char* title)
		{}

		void GLFWWindow::SetCursorVisible(bool vis)
		{}

		void GLFWWindow::SetCursorPosition(Math::TVec2<long> pos)
		{}

		void GLFWWindow::VSync(bool vsync)
		{}

		void GLFWWindow::FullScreen(bool fullscreen)
		{}

		void GLFWWindow::ImGuiNewFrame() const
		{}
		void GLFWWindow::ImGuiEndFrame() const
		{}

		std::string GLFWWindow::OpenFileSelector(const char* filter)
		{}

		std::string GLFWWindow::OpenFileSaver(const char* filter, const char* extension)
		{}

		void GLFWWindow::ChangeWorkDirectory(std::string dir)
		{}

		void GLFWWindow::TruncateWorkDirectory(std::string dir)
		{}

		void GLFWWindow::KeyPressCallback(uint keycode)
		{
			m_Keys[keycode] = true;
			Application::s_Application->OnEvent(KeyPressEvent(keycode));
		}

		void GLFWWindow::KeyReleaseCallback(uint keycode)
		{
			m_Keys[keycode] = false;
			Application::s_Application->OnEvent(KeyReleaseEvent(keycode));
		}
	
		void GLFWWindow::MousePressCallback(uint keycode)
		{
			m_Keys[keycode] = true;
			Application::s_Application->OnEvent(MousePressEvent(keycode));
		}
		
		void GLFWWindow::MouseReleaseCallback(uint keycode)
		{
			m_Keys[keycode] = false;
			Application::s_Application->OnEvent(MouseReleaseEvent(keycode));
		}
	
		void GLFWWindow::MouseMoveCallback(Math::TVec2<long> pos)
		{
			m_MousePos += pos;
			Application::s_Application->OnEvent(MouseMoveEvent(m_MousePos));
		}
	
		void GLFWWindow::WindowResizeCallback(Math::TVec2<int> size)
		{
			Application::s_Application->m_RenderAPI->Viewport(size.x, size.y);
			Application::s_Application->OnEvent(WindowResizeEvent(size));
		}
	
		void GLFWWindow::WindowFocusCallback(bool focus)
		{
			m_Focused = focus;
			Application::s_Application->OnEvent(WindowFocusEvent(focus));
		}

		void GLFWWindow::WindowCloseCallback()
		{
			Application::s_Application->Shutdown();
			Application::s_Application->OnEvent(WindowCloseEvent());
		}

		void GLFWWindow::SetWindowProperties(const WindowProperties& winprop)
		{
			m_WindowProperties = winprop;
		}

		void GLFWWindow::FocusConsole() const
		{}

		void GLFWWindow::ErrorCallback(int error, const char* description)
		{
			DD_ERR("{0}", description);
		}

		void GLFWWindow::ConfigureMonitor() {
			GLFWmonitor* primary = glfwGetPrimaryMonitor();
			int physical_width_mm, physical_height_mm;
			glfwGetMonitorPhysicalSize(primary, &physical_width_mm, &physical_height_mm);
			
			const GLFWvidmode* mode = glfwGetVideoMode(primary);
			
			int width = mode->width;
			int height = mode->height;
			
			if (width < (int) m_WindowProperties.m_Width || height < (int) m_WindowProperties.m_Height)
			{
				DD_WARN("Application resolution is more than the resolution of the screen!");
			}

			const char* name = glfwGetMonitorName(primary);
			
			DD_INFO("Monitor: {0} {1}mmx{2}mm {3}x{4}", name, physical_width_mm, physical_height_mm, width, height);


			m_WindowProperties.m_Width = m_WindowProperties.m_Width <= 0 ? width : m_WindowProperties.m_Width;
			m_WindowProperties.m_Height = m_WindowProperties.m_Height <= 0 ? height : m_WindowProperties.m_Height;
			
		}

		void GLFWWindow::CreateDeviceContext()
		{}

	}
}