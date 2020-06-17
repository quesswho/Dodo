#include "Application.h"
#include "Core/System/Timer.h"

#include "Core/System/Memory.h"

namespace Dodo {

	Application* Application::s_Application;

	Application::Application(const WindowProperties& prop)
		: m_WindowProperties(prop), m_Closed(false)
	{
		CoreInit();
		s_Application = this;
	}

	Application::~Application()
	{
		dddelete m_ThreadManager;
		dddelete m_Window;
		dddelete m_Logger;
	}

	void Application::Run()
	{
		Init();

		Timer timer;
		m_FrameTime = 0.0f;
		float elapsed = 0.0f;
		uint frames = 0;

		while (!m_Closed)
		{
			timer.Start();
			//
			Update(m_FrameTime);
			//
			m_FrameTime = timer.End();
			elapsed += m_FrameTime;
			frames++;

			if (elapsed > 1.0f)
			{
				m_FramesPerSecond = frames;
				m_FrameTimeMs = m_FrameTime / 1000.0f;
				DD_INFO("FPS: {0}, FT: {1}", m_FramesPerSecond, m_FrameTimeMs);

				frames = 0;
				elapsed = 0.0f;
			}
		}
	}

	void Application::CoreInit()
	{

		m_Logger = ddnew Logger();
		
		m_Window = ddnew Window(m_WindowProperties);
		m_TotalPhysMemGbs = (round(m_Window->m_Pcspecs.m_TotalPhysicalMemory / 1073741824.0 * 100)) / 100; // 1024^3
		m_CpuBrand = m_Window->m_Pcspecs.m_CpuBrand;
		m_NumLogicalProcessors = std::thread::hardware_concurrency();
		m_ThreadManager = ddnew ThreadManager(m_NumLogicalProcessors-1 == 0 ? 1 : m_NumLogicalProcessors - 1);

		DD_INFO("CPU: {0} {1} Logical Processors", m_CpuBrand, m_NumLogicalProcessors);
		DD_INFO("RAM: {} GBs", m_TotalPhysMemGbs);
		DD_INFO("");
	}

	void Application::Init() {}

	void Application::Update(float elapsed)
	{
		for (Layer* layer : m_Layers)
		{
			layer->Update(elapsed);
			layer->Render();
		}

		m_Window->Update();
	}

	void Application::Shutdown()
	{
		m_Closed = true;
	}

	void Application::PushLayer(Layer* layer)
	{
		m_Layers.push_back(layer);
	}

	void Application::PopLayer(Layer* layer)
	{
		const auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);

		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
		}
	}

}