#pragma once

#include <Core/Common.h>

#include <Core/Application/WindowProperties.h>
#include <Platform/WindowAPI/NativeWindowHandle.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <string>

namespace Dodo {

	namespace Platform {


		class GLFWWindow {
			private:
				WindowProperties m_WindowProperties;

				GLFWwindow* m_Handle;
			public:
				PCSpecifications m_Pcspecs;

				GLFWWindow(const WindowProperties&);
				~GLFWWindow();

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

				// TODO: The working directory stuff should defiently not be here
				std::string OpenFileSelector(const char* filter = "All\0 * .*\0");
				std::string OpenFileSaver(const char* filter = "All\0 * .*\0", const char* extension = "\0");
				void DefaultWorkDirectory() { ChangeWorkDirectory(m_MainWorkDirectory); }
				void CurrentDialogDirectory() { ChangeWorkDirectory(m_CurrentDialogDirectory); }
				void ChangeWorkDirectory(std::string dir);
				void TruncateWorkDirectory(std::string dir);
				inline const std::string GetMainWorkDirectory() const { return m_MainWorkDirectory; }
				Math::TVec2<long> m_MousePos;
				bool m_Focused;

				    

				void SetWindowProperties(const WindowProperties& winprop);

				void FocusConsole() const;
			private:
				void Init();
				static void ErrorCallback(int error, const char* description);
				static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
				static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
				static void MouseMovedCallback(GLFWwindow* window, double xpos, double ypos);
				static void WindowResizeCallback(GLFWwindow* window, int width, int height);
				static void WindowFocusCallback(GLFWwindow* window, int focused);
				static void WindowCloseCallback(GLFWwindow* window);  

				void ConfigureMonitor();
				std::string m_CurrentDialogDirectory;
				std::string m_MainWorkDirectory;
		};
	}
}

/* Printable keys */
#define DODO_KEY_SPACE              GLFW_KEY_SPACE
#define DODO_KEY_0                  GLFW_KEY_0   
#define DODO_KEY_1                  GLFW_KEY_1   
#define DODO_KEY_2                  GLFW_KEY_2   
#define DODO_KEY_3                  GLFW_KEY_3   
#define DODO_KEY_4                  GLFW_KEY_4   
#define DODO_KEY_5                  GLFW_KEY_5   
#define DODO_KEY_6                  GLFW_KEY_6   
#define DODO_KEY_7                  GLFW_KEY_7   
#define DODO_KEY_8                  GLFW_KEY_8   
#define DODO_KEY_9                  GLFW_KEY_9   
#define DODO_KEY_A                  GLFW_KEY_A   
#define DODO_KEY_B                  GLFW_KEY_B   
#define DODO_KEY_C                  GLFW_KEY_C   
#define DODO_KEY_D                  GLFW_KEY_D   
#define DODO_KEY_E                  GLFW_KEY_E   
#define DODO_KEY_F                  GLFW_KEY_F   
#define DODO_KEY_G                  GLFW_KEY_G   
#define DODO_KEY_H                  GLFW_KEY_H   
#define DODO_KEY_I                  GLFW_KEY_I   
#define DODO_KEY_J                  GLFW_KEY_J   
#define DODO_KEY_K                  GLFW_KEY_K   
#define DODO_KEY_L                  GLFW_KEY_L   
#define DODO_KEY_M                  GLFW_KEY_M   
#define DODO_KEY_N                  GLFW_KEY_N   
#define DODO_KEY_O                  GLFW_KEY_O   
#define DODO_KEY_P                  GLFW_KEY_P   
#define DODO_KEY_Q                  GLFW_KEY_Q   
#define DODO_KEY_R                  GLFW_KEY_R   
#define DODO_KEY_S                  GLFW_KEY_S   
#define DODO_KEY_T                  GLFW_KEY_T   
#define DODO_KEY_U                  GLFW_KEY_U   
#define DODO_KEY_V                  GLFW_KEY_V   
#define DODO_KEY_W                  GLFW_KEY_W   
#define DODO_KEY_X                  GLFW_KEY_X   
#define DODO_KEY_Y                  GLFW_KEY_Y   
#define DODO_KEY_Z                  GLFW_KEY_Z   
#define DODO_KEY_KP_0               GLFW_KEY_KP_0
#define DODO_KEY_KP_1               GLFW_KEY_KP_1
#define DODO_KEY_KP_2               GLFW_KEY_KP_2
#define DODO_KEY_KP_3               GLFW_KEY_KP_3
#define DODO_KEY_KP_4               GLFW_KEY_KP_4
#define DODO_KEY_KP_5               GLFW_KEY_KP_5
#define DODO_KEY_KP_6               GLFW_KEY_KP_6
#define DODO_KEY_KP_7               GLFW_KEY_KP_7
#define DODO_KEY_KP_8               GLFW_KEY_KP_8
#define DODO_KEY_KP_9               GLFW_KEY_KP_9
#define DODO_KEY_KP_MULTIPLY        GLFW_KEY_KP_MULTIPLY  
#define DODO_KEY_KP_ADD             GLFW_KEY_KP_ADD       
#define DODO_KEY_KP_DECIMAL         GLFW_KEY_KP_DECIMAL   
#define DODO_KEY_KP_SUBTRACT        GLFW_KEY_KP_SUBTRACT  
#define DODO_KEY_KP_DIVIDE          GLFW_KEY_KP_DIVIDE    
#define DODO_KEY_SEMICOLON          GLFW_KEY_SEMICOLON      /* ; */
#define DODO_KEY_EQUAL              GLFW_KEY_EQUAL          /* = */
#define DODO_KEY_KP_EQUAL           GLFW_KEY_KP_EQUAL     
#define DODO_KEY_COMMA              GLFW_KEY_COMMA          /* , */
#define DODO_KEY_MINUS              GLFW_KEY_MINUS          /* - */
#define DODO_KEY_PERIOD             GLFW_KEY_PERIOD         /* . */
#define DODO_KEY_SLASH              GLFW_KEY_SLASH          /* / */
#define DODO_KEY_GRAVE_ACCENT       GLFW_KEY_GRAVE_ACCENT   /* ` */
#define DODO_KEY_LEFT_BRACKET       GLFW_KEY_LEFT_BRACKET   /* [ */
#define DODO_KEY_BACKSLASH          GLFW_KEY_BACKSLASH      /* \ */
#define DODO_KEY_RIGHT_BRACKET      GLFW_KEY_RIGHT_BRACKET  /* ] */
#define DODO_KEY_APOSTROPHE         GLFW_KEY_APOSTROPHE     /* ' */

/* Function keys */
#define DODO_KEY_BACKSPACE          GLFW_KEY_BACKSPACE      
#define DODO_KEY_TAB                GLFW_KEY_TAB            
#define DODO_KEY_ENTER              GLFW_KEY_ENTER          	
#define DODO_KEY_PAUSE              GLFW_KEY_PAUSE          
#define DODO_KEY_CAPS_LOCK          GLFW_KEY_CAPS_LOCK      
#define DODO_KEY_ESCAPE             GLFW_KEY_ESCAPE         
#define DODO_KEY_KP_ENTER           GLFW_KEY_KP_ENTER       
#define DODO_KEY_PAGE_UP            GLFW_KEY_PAGE_UP        
#define DODO_KEY_PAGE_DOWN          GLFW_KEY_PAGE_DOWN      
#define DODO_KEY_END                GLFW_KEY_END            
#define DODO_KEY_HOME               GLFW_KEY_HOME           
#define DODO_KEY_LEFT               GLFW_KEY_LEFT           
#define DODO_KEY_UP                 GLFW_KEY_UP             
#define DODO_KEY_RIGHT              GLFW_KEY_RIGHT          
#define DODO_KEY_DOWN               GLFW_KEY_DOWN           
#define DODO_KEY_PRINT_SCREEN       GLFW_KEY_PRINT_SCREEN   
#define DODO_KEY_INSERT             GLFW_KEY_INSERT         
#define DODO_KEY_DELETE             GLFW_KEY_DELETE         
#define DODO_KEY_LEFT_SUPER         GLFW_KEY_LEFT_SUPER     
#define DODO_KEY_LEFT_WINDOWS       GLFW_KEY_LEFT_WINDOWS   EY_LEFT_SUPER
#define DODO_KEY_RIGHT_SUPER        GLFW_KEY_RIGHT_SUPER    
#define DODO_KEY_RIGHT_WINDOWS      GLFW_KEY_RIGHT_WINDOWS  EY_RIGHT_SUPER
#define DODO_KEY_MENU               GLFW_KEY_MENU           
#define DODO_KEY_LAST               GLFW_KEY_LAST           EY_MENU
#define DODO_KEY_F1                 GLFW_KEY_F1             
#define DODO_KEY_F2                 GLFW_KEY_F2             
#define DODO_KEY_F3                 GLFW_KEY_F3             
#define DODO_KEY_F4                 GLFW_KEY_F4             
#define DODO_KEY_F5                 GLFW_KEY_F5             
#define DODO_KEY_F6                 GLFW_KEY_F6             
#define DODO_KEY_F7                 GLFW_KEY_F7             
#define DODO_KEY_F8                 GLFW_KEY_F8             
#define DODO_KEY_F9                 GLFW_KEY_F9             
#define DODO_KEY_F10                GLFW_KEY_F10            
#define DODO_KEY_F11                GLFW_KEY_F11            
#define DODO_KEY_F12                GLFW_KEY_F12            
#define DODO_KEY_F13                GLFW_KEY_F13            
#define DODO_KEY_F14                GLFW_KEY_F14            
#define DODO_KEY_F15                GLFW_KEY_F15            
#define DODO_KEY_F16                GLFW_KEY_F16            
#define DODO_KEY_F17                GLFW_KEY_F17            
#define DODO_KEY_F18				GLFW_KEY_F18			
#define DODO_KEY_F19				GLFW_KEY_F19			
#define DODO_KEY_F20				GLFW_KEY_F20			
#define DODO_KEY_F21				GLFW_KEY_F21			
#define DODO_KEY_F22				GLFW_KEY_F22			
#define DODO_KEY_F23				GLFW_KEY_F23			
#define DODO_KEY_F24				GLFW_KEY_F24			
#define DODO_KEY_NUM_LOCK           GLFW_KEY_NUM_LOCK       
#define DODO_KEY_SCROLL_LOCK        GLFW_KEY_SCROLL_LOCK    
#define DODO_KEY_LEFT_SHIFT         GLFW_KEY_LEFT_SHIFT     
#define DODO_KEY_RIGHT_SHIFT        GLFW_KEY_RIGHT_SHIFT    
#define DODO_KEY_LEFT_CONTROL       GLFW_KEY_LEFT_CONTROL   
#define DODO_KEY_RIGHT_CONTROL      GLFW_KEY_RIGHT_CONTROL  
#define DODO_KEY_LEFT_ALT           GLFW_KEY_LEFT_ALT       
#define DODO_KEY_RIGHT_ALT          GLFW_KEY_RIGHT_ALT      

/* Mouse Buttons */
#define DODO_MOUSE_BUTTON_1         GLFW_MOUSE_BUTTON_1      
#define DODO_MOUSE_BUTTON_2         GLFW_MOUSE_BUTTON_2      
#define DODO_MOUSE_BUTTON_3         GLFW_MOUSE_BUTTON_3      
#define DODO_MOUSE_BUTTON_4         GLFW_MOUSE_BUTTON_4      
#define DODO_MOUSE_BUTTON_5         GLFW_MOUSE_BUTTON_5      
#define DODO_MOUSE_BUTTON_6         GLFW_MOUSE_BUTTON_6      
#define DODO_MOUSE_BUTTON_7         GLFW_MOUSE_BUTTON_7      
#define DODO_MOUSE_BUTTON_8         GLFW_MOUSE_BUTTON_8     
#define DODO_MOUSE_BUTTON_LAST      GLFW_MOUSE_BUTTON_LAST   
#define DODO_MOUSE_BUTTON_LEFT      GLFW_MOUSE_BUTTON_LEFT   
#define DODO_MOUSE_BUTTON_RIGHT     GLFW_MOUSE_BUTTON_RIGHT  
#define DODO_MOUSE_BUTTON_MIDDLE    GLFW_MOUSE_BUTTON_MIDDLE 