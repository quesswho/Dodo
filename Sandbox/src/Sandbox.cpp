#include "SandBox.h"

#include <imgui_impl_win32.h>
#include <imgui_impl_opengl3.h>
using namespace Dodo;

GameLayer::GameLayer()
{
	Application::s_Application->m_RenderAPI->ClearColor(0.2f, 0.2f, 0.9f);
	//ImGui::ShowDemoWindow();
}
GameLayer::~GameLayer()
{

}

void GameLayer::Update(float elapsed)
{

}

void GameLayer::Render()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::ShowDemoWindow();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GameLayer::OnEvent(const Event& event)
{
	switch (event.GetType())
	{
		case EventType::KEY_PRESSED:
			if (static_cast<const KeyPressEvent&>(event).m_Key == DODO_KEY_ESCAPE)
				Application::s_Application->Shutdown();
			break;
		case EventType::MOUSE_PRESSED:
			break;
		case EventType::MOUSE_POSITION:
			const Math::TVec2<long> mousepos = static_cast<const MouseMoveEvent&>(event).m_MousePos;
	}
}



class Sandbox : public Application
{
public:

	Sandbox()
		: Application(PreInit())
	{}

	WindowProperties PreInit()
	{
		WindowProperties props;
		props.m_Title = "SandBox";
		props.m_Width = 1080;
		props.m_Height = 720;
		props.m_Fullscreen = false;
		props.m_Vsync = false;
		return props;
	}

	void Init()
	{
		PushLayer(new GameLayer());
	}

};

int main() {
	Sandbox* sandBox = new Sandbox();
	sandBox->Run();
	delete sandBox;
}