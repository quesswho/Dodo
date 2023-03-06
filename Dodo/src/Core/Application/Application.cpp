#include "pch.h"
#include "Application.h"

#include "Core/System/Timer.h"

namespace Dodo {

	Application* Application::s_Application;

	Application::Application(const WindowProperties& prop)
		: m_WindowProperties(prop), m_Closed(false), m_Initializing(true), m_FramesPerSecond(0), m_FrameTimeMs(0)
	{
		s_Application = this;
		CoreInit();
	}

	Application::~Application()
	{
		dddelete m_ThreadManager;
		dddelete m_Window;
		dddelete m_Logger;
		dddelete m_RenderAPI;
	}

	void Application::Run()
	{
		Init();

		Timer timer;
		m_FrameTime = 0.0f;
		float elapsed = 0.0f;
		uint frames = 0;

		m_ThreadManager->WaitMain();
		m_Initializing = false;
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
				m_FrameTimeMs = m_FrameTime * 1000.0f;
				DD_INFO("FPS: {0}, FT: {1:.4f}", m_FramesPerSecond, m_FrameTimeMs);

				frames = 0;
				elapsed = 0.0f;
			}
		}
	}

	void Application::CoreInit()
	{

		m_Logger = ddnew Logger();
		
		m_Window = ddnew Window(m_WindowProperties);
		m_TotalPhysMemGbs = round(m_Window->m_Pcspecs.m_TotalPhysicalMemory / 1073741824.0f * 100) / 100; // 1024^3
		m_CpuBrand = m_Window->m_Pcspecs.m_CpuBrand;
		m_NumLogicalProcessors = std::thread::hardware_concurrency();
		m_ThreadManager = ddnew ThreadManager(m_NumLogicalProcessors-1 == 0 ? 1 : m_NumLogicalProcessors - 1);

		m_RenderAPI = ddnew RenderAPI();
		int res = m_RenderAPI->Init(m_WindowProperties);
		if(res)
		{
			DD_INFO("CPU: {0} {1} Logical Processors", m_CpuBrand, m_NumLogicalProcessors);
			DD_INFO("GPU: {}", Application::m_RenderAPI->m_GPUInfo);
			DD_INFO("RAM: {}", StringUtils::GigaByte(m_TotalPhysMemGbs));
		}
		else
		{
			DD_FATAL("Could not initialize RenderAPI. Error: {}", res);
		}

		m_AssetManager = ddnew AssetManager(m_WindowProperties.m_Flags & DodoWindowFlags_SERIALIZESCENE);
	}

	void Application::Init() {}

	void Application::Update(float elapsed)
	{
		m_RenderAPI->Begin();
		
		for (auto& it = m_Layers.begin(); it != m_Layers.end(); it++)
		{
			(*it)->Update(elapsed);
			(*it)->Render();
		}

		m_Window->Update();
	}

	void Application::OnEvent(const Event& event)
	{
		for (auto& it = m_Layers.rbegin(); it != m_Layers.rend(); ++it)
		{
			if (event.m_Handled)
				break;

			(*it)->OnEvent(event);
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

	void Application::ImGuiNewFrame()
	{
		m_RenderAPI->ImGuiNewFrame();
		m_Window->ImGuiNewFrame();
	}

	void Application::ImGuiEndFrame()
	{
		m_Window->ImGuiEndFrame();
		m_RenderAPI->ImGuiEndFrame();
	}
}