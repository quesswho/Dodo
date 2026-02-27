#pragma once

#include "pch.h"

#include "Window.h"
#include "Input/InputManager.h"
#include "Layer.h"

#include "Event.h"

#include "Core/Graphics/RenderAPI.h"
#include "Core/Data/AssetManager.h"
#include "Core/System/ThreadManager.h"

namespace Dodo {

	class Application {
	public:
		static Application* s_Application;

		Application(const WindowProperties&);
		virtual ~Application();

		void CoreInit();
		void Run(); // Main loop is contained in here
		void Shutdown();

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);

		void OnEvent(const Event& event);

		void ImGuiNewFrame();
		void ImGuiEndFrame();

		virtual void Init();
		virtual void Update(float elapsed);
		
		const Input& GetInput() const {
			return m_InputManager.GetInput();
		}
	public:
		float m_FrameTimeMs;
		uint m_FramesPerSecond;
		WindowProperties m_WindowProperties;

		InputManager m_InputManager;
		Logger* m_Logger;
		Window* m_Window;
		RenderAPI* m_RenderAPI;
		ThreadManager* m_ThreadManager;
		AssetManager* m_AssetManager;
		bool m_Initializing;
	private:
		double m_TotalPhysMemGbs;
		std::string m_CpuBrand;

		std::vector<Layer*> m_Layers;

		float m_FrameTime;
		bool m_Closed;
		uint m_NumLogicalProcessors;
	};
}