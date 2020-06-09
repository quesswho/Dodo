#include "Application.h"
#include "Core/System/Timer.h"

namespace Dodo {

	Application* Application::s_Application;

	Application::Application(const ApplicationProperties& prop)
		: m_ApplicationProps(prop), m_Closed(false)
	{
		CoreInit();
		s_Application = this;
	}

	Application::~Application()
	{
		delete m_ThreadManager;
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
				frames = 0;
				elapsed = 0.0f;
			}
		}
	}

	void Application::CoreInit()
	{
		m_NumThreads = std::thread::hardware_concurrency();
		m_ThreadManager = new ThreadManager(m_NumThreads-1 == 0 ? 1 : m_NumThreads - 1);
	}

	void Application::Init() {}

	void Application::Update(float elapsed)
	{
		for (Layer* layer : m_Layers)
		{
			layer->Update(elapsed);
			layer->Render();
		}
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