#pragma once

#include "pch.h"
#include "Core/System/ThreadManager.h"

#include "Window.h"
#include "Layer.h"

#include "Event.h"

#include "Core/Graphics/RenderAPI.h"

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

		virtual void Init();
		virtual void Update(float elapsed);
	public:
		float m_FrameTimeMs;
		uint m_FramesPerSecond;

		Logger* m_Logger;
		Window* m_Window;
		RenderAPI* m_RenderAPI;
		ThreadManager* m_ThreadManager;

		bool m_Initializing;
	private:
		double m_TotalPhysMemGbs;
		std::string m_CpuBrand;

		std::vector<Layer*> m_Layers;

		WindowProperties m_WindowProperties;

		float m_FrameTime;
		bool m_Closed;
		uint m_NumLogicalProcessors;
	};
}