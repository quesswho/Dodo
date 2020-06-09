#pragma once

#include "Core/Common/Common.h"

#include <vector>

#include "Layer.h"

namespace Dodo {

	struct ApplicationProperties
	{
		const char* m_Title;
		uint m_Width;
		uint m_Height;
		
		// Terrible default constructor so people are forced to change it through PreInit()
		ApplicationProperties()
		{
			m_Title = "dOdO eNgIn3";
			m_Width = 400;
			m_Height = 400;
		}
	};

	class Application {
	public:
		static Application* s_Application;

		Application(const ApplicationProperties&);
		virtual ~Application();

		void Run(); // Main loop is contained in here
		void CoreInit();
		void Shutdown();

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);

		virtual void Init() = 0; // Called after the window is created
		virtual void Update(float elapsed);
	private:
		std::vector<Layer*> m_Layers;

		bool m_Closed;
		ApplicationProperties m_ApplicationProps;
	};
}