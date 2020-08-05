#pragma once
#include "Core/Common.h"

#include <string>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#undef NOGDI
#include <Windows.h>
#define NOGDI
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace Dodo {

	enum WindowFlags
	{
		DodoWindowFlags_NONE			= 0,
		DodoWindowFlags_FULLSCREEN		= 1 << 0,
		DodoWindowFlags_VSYNC			= 1 << 1,
		DodoWindowFlags_IMGUI			= 1 << 2,
		DodoWindowFlags_IMGUIDOCKING	= 1 << 3,
		DodoWindowFlags_BACKFACECULL	= 1 << 4,
		DodoWindowFlags_SERIALIZESCENE  = 1 << 5,
	};
#pragma warning(push)
#pragma warning(disable : 26812)
	DEFINE_ENUM_FLAG_OPERATORS(WindowFlags);
#pragma warning(pop)

	struct WindowProperties {
		WindowProperties() : m_Width(0), m_Height(0), m_Title("Dodo Engine"), m_Flags(DodoWindowFlags_NONE) {}
		uint m_Width;
		uint m_Height;
		const char* m_Title;

		WindowFlags m_Flags;
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
			void SetCursorPosition(Math::TVec2<long> pos);
			void SetCursorVisible(bool vis);
			void VSync(bool vsync);
			void FullScreen(bool fullscreen);
			void FullScreen() { FullScreen(~m_WindowProperties.m_Flags & DodoWindowFlags_FULLSCREEN); }
			void ImGuiNewFrame() const;
			void ImGuiEndFrame() const;

			std::string OpenFileDialog();
			void DefaultWorkDirectory() { ChangeWorkDirectory(m_MainWorkDirectory); }
			void CurrentDialogDirector() { ChangeWorkDirectory(m_CurrentDialogDirectory); }
			void ChangeWorkDirectory(std::string dir);
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

/* Printable keys */
#define DODO_KEY_SPACE              0x20
#define DODO_KEY_0                  0x30
#define DODO_KEY_1                  0x31
#define DODO_KEY_2                  0x32
#define DODO_KEY_3                  0x33
#define DODO_KEY_4                  0x34
#define DODO_KEY_5                  0x35
#define DODO_KEY_6                  0x36
#define DODO_KEY_7                  0x37
#define DODO_KEY_8                  0x38
#define DODO_KEY_9                  0x39
#define DODO_KEY_A                  0x41
#define DODO_KEY_B                  0x42
#define DODO_KEY_C                  0x43
#define DODO_KEY_D                  0x44
#define DODO_KEY_E                  0x45
#define DODO_KEY_F                  0x46
#define DODO_KEY_G                  0x47
#define DODO_KEY_H                  0x48
#define DODO_KEY_I                  0x49
#define DODO_KEY_J                  0x4A
#define DODO_KEY_K                  0x4B
#define DODO_KEY_L                  0x4C
#define DODO_KEY_M                  0x4D
#define DODO_KEY_N                  0x4E
#define DODO_KEY_O                  0x4F
#define DODO_KEY_P                  0x50
#define DODO_KEY_Q                  0x51
#define DODO_KEY_R                  0x52
#define DODO_KEY_S                  0x53
#define DODO_KEY_T                  0x54
#define DODO_KEY_U                  0x55
#define DODO_KEY_V                  0x56
#define DODO_KEY_W                  0x57
#define DODO_KEY_X                  0x58
#define DODO_KEY_Y                  0x59
#define DODO_KEY_Z                  0x5A
#define DODO_KEY_KP_0               0x60
#define DODO_KEY_KP_1               0x61
#define DODO_KEY_KP_2               0x62
#define DODO_KEY_KP_3               0x63
#define DODO_KEY_KP_4               0x64
#define DODO_KEY_KP_5               0x65
#define DODO_KEY_KP_6               0x66
#define DODO_KEY_KP_7               0x67
#define DODO_KEY_KP_8               0x68
#define DODO_KEY_KP_9               0x69
#define DODO_KEY_KP_MULTIPLY        0x6A
#define DODO_KEY_KP_ADD             0x6B
#define DODO_KEY_KP_DECIMAL         0x6C
#define DODO_KEY_KP_SUBTRACT        0x6D
#define DODO_KEY_KP_DIVIDE          0x6F
#define DODO_KEY_SEMICOLON          0xBA  /* ; */
#define DODO_KEY_EQUAL              0xBB  /* = */
#define DODO_KEY_KP_EQUAL           DODO_KEY_EQUAL
#define DODO_KEY_COMMA              0xBC  /* , */
#define DODO_KEY_MINUS              0xBD  /* - */
#define DODO_KEY_PERIOD             0xBE  /* . */
#define DODO_KEY_SLASH              0xBF  /* / */
#define DODO_KEY_GRAVE_ACCENT       0xC0  /* ` */
#define DODO_KEY_LEFT_BRACKET       0xDB  /* [ */
#define DODO_KEY_BACKSLASH          0xDC  /* \ */
#define DODO_KEY_RIGHT_BRACKET      0xDD  /* ] */
#define DODO_KEY_APOSTROPHE         0xDE  /* ' */

/* Function keys */
#define DODO_KEY_BACKSPACE          0x08
#define DODO_KEY_TAB                0x09
#define DODO_KEY_ENTER              0x0D
#define DODO_KEY_CONTROL			0x11
#define DODO_KEY_PAUSE              0x13
#define DODO_KEY_CAPS_LOCK          0x14
#define DODO_KEY_ESCAPE             0x1B
#define DODO_KEY_KP_ENTER           0x1C
#define DODO_KEY_PAGE_UP            0x21
#define DODO_KEY_PAGE_DOWN          0x22
#define DODO_KEY_END                0x23
#define DODO_KEY_HOME               0x24
#define DODO_KEY_LEFT               0x25
#define DODO_KEY_UP                 0x26
#define DODO_KEY_RIGHT              0x27
#define DODO_KEY_DOWN               0x28
#define DODO_KEY_PRINT_SCREEN       0x2C
#define DODO_KEY_INSERT             0x2D
#define DODO_KEY_DELETE             0x2E
#define DODO_KEY_LEFT_SUPER         0x5B
#define DODO_KEY_LEFT_WINDOWS       DODO_KEY_LEFT_SUPER
#define DODO_KEY_RIGHT_SUPER        0x5C
#define DODO_KEY_RIGHT_WINDOWS      DODO_KEY_RIGHT_SUPER
#define DODO_KEY_MENU               0x5D
#define DODO_KEY_LAST               DODO_KEY_MENU
#define DODO_KEY_F1                 0x70
#define DODO_KEY_F2                 0x71
#define DODO_KEY_F3                 0x72
#define DODO_KEY_F4                 0x73
#define DODO_KEY_F5                 0x74
#define DODO_KEY_F6                 0x75
#define DODO_KEY_F7                 0x76
#define DODO_KEY_F8                 0x77
#define DODO_KEY_F9                 0x78
#define DODO_KEY_F10                0x79
#define DODO_KEY_F11                0x7A
#define DODO_KEY_F12                0x7B
#define DODO_KEY_F13                0x7C
#define DODO_KEY_F14                0x7D
#define DODO_KEY_F15                0x7E
#define DODO_KEY_F16                0x7F
#define DODO_KEY_F17                0x80
#define DODO_KEY_F18				0x81
#define DODO_KEY_F19				0x82
#define DODO_KEY_F20				0x83
#define DODO_KEY_F21				0x84
#define DODO_KEY_F22				0x85
#define DODO_KEY_F23				0x86
#define DODO_KEY_F24				0x87
#define DODO_KEY_NUM_LOCK           0x90
#define DODO_KEY_SCROLL_LOCK        0x91
#define DODO_KEY_LEFT_SHIFT         0xA0
#define DODO_KEY_RIGHT_SHIFT        0xA1
#define DODO_KEY_LEFT_CONTROL       0xA2
#define DODO_KEY_RIGHT_CONTROL      0xA3
#define DODO_KEY_LEFT_ALT           0xA4
#define DODO_KEY_RIGHT_ALT          0xA5

/* Mouse Buttons */
#define CROW_MOUSE_BUTTON_1         0x01
#define CROW_MOUSE_BUTTON_2         0x02
#define CROW_MOUSE_BUTTON_3         0x04
#define CROW_MOUSE_BUTTON_4         0x05
#define CROW_MOUSE_BUTTON_5         0x06
#define CROW_MOUSE_BUTTON_LEFT      CROW_MOUSE_BUTTON_1
#define CROW_MOUSE_BUTTON_RIGHT     CROW_MOUSE_BUTTON_2
#define CROW_MOUSE_BUTTON_MIDDLE    CROW_MOUSE_BUTTON_3