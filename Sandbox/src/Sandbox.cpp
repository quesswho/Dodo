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



class Sandbox : public Application
{
public:

	Sandbox()
		: Application(PreInit())
	{}

	ApplicationProperties& PreInit()
	{
		ApplicationProperties props;
		props.m_Title = "SandBox";
		props.m_Width = 1080;
		props.m_Height = 720;
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