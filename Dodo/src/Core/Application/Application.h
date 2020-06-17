#pragma once

#include "Core/Common/Common.h"
#include "Core/System/ThreadManager.h"

#include "Window.h"
#include "Layer.h"
#include "Core/Utilities/Logger.h"

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

		virtual void Init();
		virtual void Update(float elapsed);
	public:
		float m_FrameTimeMs;
		uint m_FramesPerSecond;

		Logger* m_Logger;
	private:
		double m_TotalPhysMemGbs;
		std::string m_CpuBrand;

		Window* m_Window;
		std::vector<Layer*> m_Layers;
		ThreadManager* m_ThreadManager;


		WindowProperties m_WindowProperties;

		float m_FrameTime;
		bool m_Closed;
		uint m_NumLogicalProcessors;
	};
}