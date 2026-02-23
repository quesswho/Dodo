#include "pch.h"
#include "GLFWWindow.h"

namespace Dodo {

	namespace Platform {

		GLFWWindow::GLFWWindow(const WindowProperties& winProp)
			: m_WindowProperties(winProp), m_Focused(true)
		{
			Init();
		}

		GLFWWindow::~GLFWWindow()
		{}

		void GLFWWindow::Init()
		{}

		void GLFWWindow::Update() const
		{}

		void GLFWWindow::FocusConsole() const
		{}
	}
}