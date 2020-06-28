#include "SandBox.h"

using namespace Dodo;

GameLayer::GameLayer()
{

}
GameLayer::~GameLayer()
{

}

void GameLayer::Update(float elapsed)
{

}

void GameLayer::Render()
{

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
			const Math::TVec2<double> mousepos = static_cast<const MouseMoveEvent&>(event).m_MousePos;
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