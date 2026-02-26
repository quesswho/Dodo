#pragma once

#include <Core/Common.h>

#include <Core/Application/WindowProperties.h>
#include <Platform/WindowAPI/NativeWindowHandle.h>

#include <string>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#undef NOGDI
#include <Windows.h>
#define NOGDI
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace Dodo {
	namespace Platform {


		class Win32Window {
		private:
			WindowProperties m_WindowProperties;

			HINSTANCE m_HInstance;
			HWND m_Hwnd;
			HDC m_Hdc;
		public:
			PCSpecifications m_Pcspecs;

			Win32Window(const WindowProperties&);
			~Win32Window();

			void Update() const;
			void SetTitle(const char* title);
			void SetCursorPosition(Math::TVec2<long> pos);
			void SetCursorVisible(bool vis);
			void VSync(bool vsync);
			void FullScreen(bool fullscreen);
			void FullScreen() { FullScreen(~m_WindowProperties.m_Flags & DodoWindowFlags_FULLSCREEN); }
			NativeWindowHandle GetHandle() const;
			void ImGuiNewFrame() const;
			void ImGuiEndFrame() const;

			std::string OpenFileSelector(const char* filter = "All\0 * .*\0");
			std::string OpenFileSaver(const char* filter = "All\0 * .*\0", const char* extension = "\0");
			void DefaultWorkDirectory() { ChangeWorkDirectory(m_MainWorkDirectory); }
			void CurrentDialogDirectory() { ChangeWorkDirectory(m_CurrentDialogDirectory); }
			void ChangeWorkDirectory(std::string dir);
			void TruncateWorkDirectory(std::string dir);
			inline const std::string GetMainWorkDirectory() const { return m_MainWorkDirectory; }
			bool m_Keys[1024];
			Math::TVec2<long> m_MousePos;
			bool m_Focused;

			void KeyPressCallback(uint keycode);
			void KeyReleaseCallback(uint keycode);
			void MousePressCallback(uint keycode);
			void MouseReleaseCallback(uint keycode);
			void MouseMoveCallback(Math::TVec2<long> pos);
			void WindowResizeCallback(Math::TVec2<int> size);
			void WindowFocusCallback(bool focused);
			void WindowCloseCallback();      

			void SetWindowProperties(const WindowProperties& winprop);

			void FocusConsole() const;
		private:
			void Init();
			void RegisterRawMouse() const;
			PIXELFORMATDESCRIPTOR GetPixelFormat() const;
			short int CreateDeviceContext();
			
			std::string m_CurrentDialogDirectory;
			std::string m_MainWorkDirectory;
		};
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static Win32Window* s_WindowClass;
	}
}