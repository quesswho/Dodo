#include "Application.h"

namespace Dodo {

	Application* Application::s_Application;

	Application::Application(const ApplicationProperties& prop)
		: m_ApplicationProps(prop), m_Closed(false)
	{
		s_Application = this;
	}

	Application::~Application()
	{}

	void Application::Run()
	{
		CoreInit();
		Init();
		while (!m_Closed)
		{
			Update(0.0f);
		}
	}

	void Application::CoreInit()
	{

	}

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