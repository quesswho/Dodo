#include "SandBox.h"

using namespace Dodo;
using namespace Math;

GameLayer::GameLayer()
{
	Application::s_Application->m_RenderAPI->ClearColor(0.2f, 0.2f, 0.9f);
	Application::s_Application->m_RenderAPI->DepthTest(true);
	Application::s_Application->m_RenderAPI->Blending(true);

	m_Camera = new FreeCamera(Vec3(0.0f, 0.0f, 20.0f), (float)Application::s_Application->m_WindowProperties.m_Width / (float)Application::s_Application->m_WindowProperties.m_Height, 0.04f, 10.0f);

	FrameBufferProperties frameprop;
	frameprop.m_Width = Application::s_Application->m_WindowProperties.m_Width;
	frameprop.m_Height = Application::s_Application->m_WindowProperties.m_Height;

	m_PostEffect = new PostEffect(frameprop, "res/shader/gamma.fx");
	m_Gamma = 1.5f;

	m_Scene = m_File.Read("res/sponza/sponza.das");
	//m_Scene = m_File.Read("res/knife.das");

	std::vector<std::string> skyboxPath = {
		"res/texture/skybox/right.jpg",
		"res/texture/skybox/left.jpg",
		"res/texture/skybox/top.jpg",
		"res/texture/skybox/bottom.jpg",
		"res/texture/skybox/front.jpg",
		"res/texture/skybox/back.jpg",
	};

	m_Scene->m_SkyBox = new Skybox(m_Camera->GetProjectionMatrix(), skyboxPath);
	m_Scene->m_LightSystem.m_Directional.m_Direction = Normalize(Vec3(0.2f, -0.5f, -0.5f));

	Application::s_Application->m_Window->SetCursorVisible(false);
	m_Camera->ResetMouse();
}
GameLayer::~GameLayer()
{
	Application::s_Application->m_Window->DefaultWorkDirectory();
	delete m_PostEffect;
	delete m_Camera;
	delete m_Scene;
}

void GameLayer::Update(float elapsed)
{
	if (Application::s_Application->m_Window->m_Keys[DODO_KEY_UP])
		m_Gamma += 1.0f * elapsed;
	if (Application::s_Application->m_Window->m_Keys[DODO_KEY_DOWN])
		m_Gamma -= 1.0f * elapsed;

	// Change directional light
	if (Application::s_Application->m_Window->m_Keys[DODO_KEY_1])
		m_Scene->m_LightSystem.m_Directional.m_Direction += elapsed * Vec3(1.0f, 0, 0);
	if (Application::s_Application->m_Window->m_Keys[DODO_KEY_2])
		m_Scene->m_LightSystem.m_Directional.m_Direction -= elapsed * Vec3(1.0f, 0, 0);
	if (Application::s_Application->m_Window->m_Keys[DODO_KEY_3])
		m_Scene->m_LightSystem.m_Directional.m_Direction += elapsed * Vec3(0, 1.0f, 0);
	if (Application::s_Application->m_Window->m_Keys[DODO_KEY_4])
		m_Scene->m_LightSystem.m_Directional.m_Direction -= elapsed * Vec3(0, 1.0f, 0);
	if (Application::s_Application->m_Window->m_Keys[DODO_KEY_5])
		m_Scene->m_LightSystem.m_Directional.m_Direction += elapsed * Vec3(0, 0, 1.0f);
	if (Application::s_Application->m_Window->m_Keys[DODO_KEY_6])
		m_Scene->m_LightSystem.m_Directional.m_Direction -= elapsed * Vec3(0, 0, 1.0f);
	if (Application::s_Application->m_Window->m_Keys[DODO_KEY_0])
		m_Scene->m_LightSystem.m_Directional.m_Direction = Vec3(0.0f, -1.0f, 1.0f);
	m_Scene->m_LightSystem.m_Directional.m_Direction = Normalize(m_Scene->m_LightSystem.m_Directional.m_Direction);

	m_PostEffect->SetUniformValue("u_Gamma", m_Gamma);

	m_Camera->Update(elapsed);
}

void GameLayer::Render()
{
	m_PostEffect->Bind();

	m_Scene->UpdateCamera(m_Camera);
	m_Scene->Draw();
	 
	m_PostEffect->Draw();
}

void GameLayer::OnEvent(const Event& event)
{
	switch (event.GetType())
	{
		case EventType::KEY_PRESSED:
			if (static_cast<const KeyPressEvent&>(event).m_Key == DODO_KEY_F11) 
			{
				Application::s_Application->m_Window->FullScreen();
				m_Camera->Resize(Application::s_Application->m_WindowProperties.m_Width, Application::s_Application->m_WindowProperties.m_Height);
				m_PostEffect->Resize(Application::s_Application->m_WindowProperties.m_Width, Application::s_Application->m_WindowProperties.m_Height);
			}
				
			if (static_cast<const KeyPressEvent&>(event).m_Key == DODO_KEY_ESCAPE) 
			{
				Application::s_Application->Shutdown();
			}
			break;
		case EventType::MOUSE_PRESSED:
			break;
		case EventType::MOUSE_POSITION:
			m_Camera->UpdateRotation();
			break;
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
		props.m_Flags = DodoWindowFlags_BACKFACECULL;
		//props.m_Flags = DodoWindowFlags_NONE;
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