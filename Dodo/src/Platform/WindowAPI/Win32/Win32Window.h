#pragma once
#include "Core/Common/Common.h"
#include <string>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#undef NOGDI
#include <Windows.h>
#define NOGDI
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace Dodo {

	struct WindowProperties {
		uint m_Width;
		uint m_Height;
		const char* m_Title;

		bool m_Fullscreen;
	};

	struct PCSpecifications {
		size_t m_TotalPhysicalMemory;
		std::string m_CpuBrand;
	};

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
			void SetCursorVisibility(bool vis);

		private:
			void Init();
		};

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static Win32Window* s_WindowClass;
	}
}