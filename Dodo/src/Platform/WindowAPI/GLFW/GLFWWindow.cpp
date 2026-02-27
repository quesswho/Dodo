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
			glfwDestroyWindow(m_Handle);
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
			m_Handle = glfwCreateWindow(m_WindowProperties.m_Width, 
				m_WindowProperties.m_Height, 
				m_WindowProperties.m_Title, 
				NULL, NULL);
			if (!m_Handle)
			{
				DD_FATAL("Could not create GLFW window!");
				return;
			}
			int winW, winH, fbW, fbH;
			glfwGetWindowSize(m_Handle, &winW, &winH);
			glfwGetFramebufferSize(m_Handle, &fbW, &fbH);
			DD_INFO("Window: {}x{}, Framebuffer: {}x{}", winW, winH, fbW, fbH);
			
			glfwSetWindowUserPointer(m_Handle, this);
			glfwSetKeyCallback(m_Handle, KeyCallback);
			glfwSetMouseButtonCallback(m_Handle, MouseButtonCallback);
			// TODO: Raw mouse https://www.glfw.org/docs/latest/input_guide.html
			glfwSetCursorPosCallback(m_Handle, MouseMovedCallback);
			glfwSetWindowSizeCallback(m_Handle, WindowResizeCallback);
			glfwSetFramebufferSizeCallback(m_Handle, FramebufferResizeCallback);
			glfwSetWindowFocusCallback(m_Handle, WindowFocusCallback);
			glfwSetWindowCloseCallback(m_Handle, WindowCloseCallback);
		}

		void GLFWWindow::Update() const
		{
			glfwPollEvents();
		}

		void GLFWWindow::SetTitle(const char* title)
		{
			glfwSetWindowTitle(m_Handle, title);
		}

		void GLFWWindow::SetCursorVisible(bool vis)
		{
			if (vis) {
				glfwSetInputMode(m_Handle, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
				glfwSetInputMode(m_Handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			} else {
				glfwSetInputMode(m_Handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				if (glfwRawMouseMotionSupported()) {
					glfwSetInputMode(m_Handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
				} else {
					DD_WARN("Raw Mouse is not supported!");
				}
			}
		}

		void GLFWWindow::SetCursorPosition(Math::TVec2<double> pos)
		{
			glfwSetCursorPos(m_Handle, pos.x, pos.y);
		}

		void GLFWWindow::VSync(bool vsync)
		{}

		void GLFWWindow::FullScreen(bool fullscreen)
		{}

		NativeWindowHandle GLFWWindow::GetHandle() const
		{
			return {
				NativeWindowHandle::WindowBackend::GLFW,
				(void*)m_Handle,  // GLFWwindow*
				nullptr           // No display needed
			};
		}

		void GLFWWindow::ImGuiNewFrame() const
		{}
		void GLFWWindow::ImGuiEndFrame() const
		{}

		std::string GLFWWindow::OpenFileSelector(const char* filter)
		{}

		std::string GLFWWindow::OpenFileSaver(const char* filter, const char* extension)
		{}

		void GLFWWindow::ChangeWorkDirectory(std::string dir)
		{
			
		}

		void GLFWWindow::TruncateWorkDirectory(std::string dir)
		{
		}

		void GLFWWindow::ErrorCallback(int error, const char* description)
		{
			DD_ERR("{0}", description);
		}

		
		void GLFWWindow::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			switch(action) {
				case GLFW_PRESS:
				case GLFW_REPEAT:
					Application::s_Application->m_InputManager.KeyPressed(key);
					break;
				case GLFW_RELEASE:
					Application::s_Application->m_InputManager.KeyReleased(key);
					break;
			}
		}

		
		void GLFWWindow::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
		{
			switch(action) {
				case GLFW_PRESS:
					Application::s_Application->m_InputManager.MousePressed(button);
					break;
				case GLFW_RELEASE:
					Application::s_Application->m_InputManager.MouseReleased(button);
					break;
			}
		}
		
		
	
		void GLFWWindow::MouseMovedCallback(GLFWwindow* window, double xpos, double ypos)
		{
			// TODO: support subpixel mouse
			Application::s_Application->m_InputManager.MouseMoved(Math::TVec2<double>(xpos, ypos));
		}
	
		void GLFWWindow::WindowResizeCallback(GLFWwindow* window, int width, int height)
		{
			DD_INFO("Window resize: {0}x{1}", width, height);
			Application::s_Application->OnEvent(WindowResizeEvent(Math::TVec2<int>(width, height)));
		}

		void GLFWWindow::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
		{
			DD_INFO("Framebuffer resize: {0}x{1}", width, height);
			//Application::s_Application->m_RenderAPI->Viewport(width, height);
		}
	
		void GLFWWindow::WindowFocusCallback(GLFWwindow* window, int focus)
		{
			GLFWWindow* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
			if(!self) return;
			self->m_Focused = focus > 0;
			Application::s_Application->OnEvent(WindowFocusEvent(self->m_Focused));
		}

		void GLFWWindow::WindowCloseCallback(GLFWwindow* window)
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
	}
}