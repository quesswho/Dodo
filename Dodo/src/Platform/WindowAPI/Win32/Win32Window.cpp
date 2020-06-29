#include "pch.h"
#include "Win32Window.h"

#include "Core/Application/Application.h"
#include <glad/wgl.h>

#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Dodo {

	namespace Platform {

		Win32Window::Win32Window(const WindowProperties& winProp)
			: m_WindowProperties(winProp), m_Focused(true)
		{
			Init();
		}

		Win32Window::~Win32Window()
		{
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			ShowCursor(true);
			DestroyWindow(m_Hwnd);
		}

		void Win32Window::Init()
		{
			s_WindowClass = this;
			m_HInstance = (HINSTANCE)&__ImageBase;

			WNDCLASSEX wc;
			wc.cbSize = sizeof(WNDCLASSEX);
			wc.hInstance = m_HInstance;
			wc.lpfnWndProc = WndProc;
			wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
			wc.hIconSm = wc.hIcon;
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
			wc.lpszMenuName = NULL;
			wc.lpszClassName = L"WindowsWindowClass";

			if (!RegisterClassEx(&wc))
			{
				DD_FATAL("Could not register Window class!");
			}


			if (GetSystemMetrics(SM_CXSCREEN) < (int) m_WindowProperties.m_Width || GetSystemMetrics(SM_CYSCREEN) < (int) m_WindowProperties.m_Height)
			{
				DD_WARN("Application resolution is more than the resolution of the screen!");
			}

			m_WindowProperties.m_Width = m_WindowProperties.m_Width == 0 ? GetSystemMetrics(SM_CXSCREEN) : m_WindowProperties.m_Width;
			m_WindowProperties.m_Height = m_WindowProperties.m_Height == 0 ? GetSystemMetrics(SM_CYSCREEN) : m_WindowProperties.m_Height;

			int posX, posY;

			if (m_WindowProperties.m_Fullscreen)
			{
				m_Hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, wc.lpszClassName, L"", WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
					0, 0, m_WindowProperties.m_Width, m_WindowProperties.m_Height, NULL, NULL, m_HInstance, NULL);

				posX = (GetSystemMetrics(SM_CXSCREEN) - m_WindowProperties.m_Width) / 2;
				posY = (GetSystemMetrics(SM_CYSCREEN) - m_WindowProperties.m_Height) / 2;

				SetWindowPos(m_Hwnd, HWND_TOP, posX, posY, m_WindowProperties.m_Width, m_WindowProperties.m_Height, 0);

			}
			else
			{

				m_Hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW,
					0, 0, m_WindowProperties.m_Width, m_WindowProperties.m_Height, NULL, NULL, m_HInstance, NULL);

				posX = (GetSystemMetrics(SM_CXSCREEN) - m_WindowProperties.m_Width) / 2 - (GetSystemMetrics(SM_CXSMSIZE) - GetSystemMetrics(SM_CXEDGE) - GetSystemMetrics(SM_CXFRAME)) / 2;
				posY = (GetSystemMetrics(SM_CYSCREEN) - m_WindowProperties.m_Height) / 2 - GetSystemMetrics(SM_CYCAPTION) - (GetSystemMetrics(SM_CYSMSIZE) + GetSystemMetrics(SM_CYEDGE)) / 2 + GetSystemMetrics(SM_CYFRAME);

				SetWindowPos(m_Hwnd, HWND_TOP, posX, posY, m_WindowProperties.m_Width + GetSystemMetrics(SM_CXSMSIZE) - GetSystemMetrics(SM_CXEDGE) - GetSystemMetrics(SM_CXFRAME), m_WindowProperties.m_Height + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYSMSIZE) - GetSystemMetrics(SM_CYEDGE) - GetSystemMetrics(SM_CYFRAME), 0);

			}

			if (!m_Hwnd)
			{
				DD_FATAL("Could not create window!");
				return;
			}


			
			SYSTEM_INFO sysinfo;
			GetSystemInfo(&sysinfo);

			int cpuInfo[4] = { -1 };
			char cpuName[0x40];
			__cpuid(cpuInfo, 0x80000000);
			int nExIds = cpuInfo[0];

			memset(cpuName, 0, sizeof(cpuName));

			for (int i = 0x80000000; i <= nExIds; ++i)
			{
				__cpuid(cpuInfo, i);
				if (i == 0x80000002)
					memcpy(cpuName, cpuInfo, sizeof(cpuInfo));
				else if (i == 0x80000003)
					memcpy(cpuName + 16, cpuInfo, sizeof(cpuInfo));
				else if (i == 0x80000004)
					memcpy(cpuName + 32, cpuInfo, sizeof(cpuInfo));
			}

			m_Pcspecs.m_CpuBrand = cpuName;

			std::size_t doubleSpace = m_Pcspecs.m_CpuBrand.find("  ");
			while (doubleSpace != std::string::npos) // Remove double spacing
			{
				m_Pcspecs.m_CpuBrand.erase(doubleSpace, 1);
				doubleSpace = m_Pcspecs.m_CpuBrand.find("  ");
			}


			MEMORYSTATUSEX memInfo;
			memInfo.dwLength = sizeof(MEMORYSTATUSEX);
			GlobalMemoryStatusEx(&memInfo);
			m_Pcspecs.m_TotalPhysicalMemory = memInfo.ullTotalPhys;                                           

			memset(m_Keys, 0, sizeof(m_Keys));

			RegisterRawMouse();
			CreateDeviceContext();
			
			wglSwapIntervalEXT(m_WindowProperties.m_Vsync);

			SetWindowTextA(m_Hwnd, m_WindowProperties.m_Title);

			ShowWindow(m_Hwnd, SW_SHOW);
			SetForegroundWindow(m_Hwnd);
			SetFocus(m_Hwnd);

			POINT p;
			GetCursorPos(&p);
			m_MousePos = Math::TVec2<long>(p.x, p.y);

			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGui_ImplWin32_Init(m_Hwnd);
			ImGuiIO& io = ImGui::GetIO(); (void)io;
		}

		void Win32Window::Update() const
		{
			MSG message;
			while (PeekMessage(&message, NULL, NULL, NULL, PM_REMOVE) > 0)
			{
				if (message.message == WM_QUIT)
				{
					Application::s_Application->m_Window->WindowCloseCallback(); // Keep update function const
					return;
				}
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
			SwapBuffers(m_Hdc);
		}

		void Win32Window::RegisterRawMouse() const
		{
			RAWINPUTDEVICE rid[1];
			rid[0].usUsagePage = ((USHORT)0x01);
			rid[0].usUsage = ((USHORT)0x02);
			rid[0].dwFlags = RIDEV_INPUTSINK;
			rid[0].hwndTarget = m_Hwnd;
			RegisterRawInputDevices(rid, 1, sizeof(rid[0]));
		}

		PIXELFORMATDESCRIPTOR Win32Window::GetPixelFormat() const
		{
			PIXELFORMATDESCRIPTOR result = {};
			result.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			result.nVersion = 1;
			result.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			result.iPixelType = PFD_TYPE_RGBA;
			result.cColorBits = 32;
			result.cDepthBits = 24;
			result.cStencilBits = 8;
			result.cAuxBuffers = 0;
			result.iLayerType = PFD_MAIN_PLANE;
			return result;
		}

		short int Win32Window::CreateDeviceContext()
		{
			m_Hdc = GetDC(m_Hwnd);

			PIXELFORMATDESCRIPTOR pfd = GetPixelFormat();
			int pixelFormat = ChoosePixelFormat(m_Hdc, &pfd);
			if (pixelFormat)
				if (!SetPixelFormat(m_Hdc, pixelFormat, &pfd))
					return -1;

			HGLRC hrc = wglCreateContext(m_Hdc);
			if (hrc)
				if (wglMakeCurrent(m_Hdc, hrc))
					if (!gladLoaderLoadWGL(m_Hdc))
						DD_ERR("Could not load WGL!");
					return 1;
			return -2;
		}


		LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			LRESULT result = NULL;
			if (s_WindowClass == nullptr)
				return DefWindowProc(hwnd, msg, wParam, lParam);

			if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
				return true;

			switch (msg)
			{
				case WM_ACTIVATE:
				{
					if (!HIWORD(wParam)) {} // Is minimized
					else{}

					return 0;
				}
				case WM_SYSCOMMAND:
				{
					switch (wParam)
					{
						case SC_SCREENSAVE:
						case SC_MONITORPOWER:
							return 0;
					}
					result = DefWindowProc(hwnd, msg, wParam, lParam);
				} break;
				case WM_SETFOCUS:
					if(!Application::s_Application->m_Initializing)
						Application::s_Application->m_Window->WindowFocusCallback(true);
					break;
				case WM_KILLFOCUS:
					Application::s_Application->m_Window->WindowFocusCallback(false);
					break;
				case WM_CLOSE:
				case WM_DESTROY:
					Application::s_Application->m_Window->WindowCloseCallback();
					break;
				case WM_SYSKEYDOWN:
				case WM_KEYDOWN:
					Application::s_Application->m_Window->KeyPressCallback((uint)wParam);
					break;
				case WM_SYSKEYUP:
				case WM_KEYUP:
					Application::s_Application->m_Window->KeyReleaseCallback((uint)wParam);
					break;
				case WM_MOUSEMOVE:
					break;
				case WM_INPUT:
				{
					UINT dwSize;
					GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize,
						sizeof(RAWINPUTHEADER));
					LPBYTE lpb = new BYTE[dwSize];
					if (lpb == NULL)
					{
						return 0;
					}

					GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
						lpb, &dwSize, sizeof(RAWINPUTHEADER));

					RAWINPUT* raw = (RAWINPUT*)lpb;

					if (raw->header.dwType == RIM_TYPEMOUSE)
					{
						Application::s_Application->m_Window->MouseMoveCallback(Math::TVec2<long>(raw->data.mouse.lLastX, raw->data.mouse.lLastY));
					}
					delete raw;
					break;
				}
				case WM_LBUTTONDOWN:
				case WM_RBUTTONDOWN:
				case WM_MBUTTONDOWN:
					Application::s_Application->m_Window->MousePressCallback((uint)wParam);
					break;
				case WM_LBUTTONUP:
				case WM_RBUTTONUP:
				case WM_MBUTTONUP:
					Application::s_Application->m_Window->MouseReleaseCallback((uint)wParam);
					break;
				case WM_SIZE:
					if (!Application::s_Application->m_Initializing)
						Application::s_Application->m_Window->WindowResizeCallback(Math::TVec2<int>(LOWORD(lParam), HIWORD(lParam)));
					break;
				default:
					result = DefWindowProc(hwnd, msg, wParam, lParam);
			}
			return result;
		}

		void Win32Window::SetTitle(const char* title)
		{
			m_WindowProperties.m_Title = title;
			SetWindowTextA(m_Hwnd, m_WindowProperties.m_Title);
		}

		void Win32Window::SetCursorVisibility(bool vis)
		{
			ShowCursor(vis);
		}

		void Win32Window::SetCursorPosition(Math::TVec2<long> pos)
		{
			POINT pt;
			pt.x = pos.x;
			pt.y = pos.y;
			ClientToScreen(m_Hwnd, &pt);
			SetCursorPos(pt.x, pt.y);
		}

		void Win32Window::VSync(bool vsync)
		{
			wglSwapIntervalEXT(vsync);
		}

		void Win32Window::KeyPressCallback(uint keycode)
		{
			m_Keys[keycode] = true;
			Application::s_Application->OnEvent(KeyPressEvent(keycode));
		}

		void Win32Window::KeyReleaseCallback(uint keycode)
		{
			m_Keys[keycode] = false;
			Application::s_Application->OnEvent(KeyReleaseEvent(keycode));
		}
	
		void Win32Window::MousePressCallback(uint keycode)
		{
			m_Keys[keycode] = true;
			Application::s_Application->OnEvent(MousePressEvent(keycode));
		}
		
		void Win32Window::MouseReleaseCallback(uint keycode)
		{
			m_Keys[keycode] = false;
			Application::s_Application->OnEvent(MouseReleaseEvent(keycode));
		}
	
		void Win32Window::MouseMoveCallback(Math::TVec2<long> pos)
		{
			m_MousePos += pos;
			Application::s_Application->OnEvent(MouseMoveEvent(m_MousePos));
		}
	
		void Win32Window::WindowResizeCallback(Math::TVec2<int> size)
		{
			Application::s_Application->m_RenderAPI->Viewport(size.x, size.y);
			Application::s_Application->OnEvent(WindowResizeEvent(size));
		}
	
		void Win32Window::WindowFocusCallback(bool focus)
		{
			m_Focused = focus;
			Application::s_Application->OnEvent(WindowFocusEvent(focus));
		}

		void Win32Window::WindowCloseCallback()
		{
			Application::s_Application->Shutdown();
			PostQuitMessage(0);
			Application::s_Application->OnEvent(WindowCloseEvent());
		}
	}
}