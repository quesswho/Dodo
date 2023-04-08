#include "Game.h"

using namespace Dodo;
using namespace Math;

GameLayer::GameLayer()
{
	Application::s_Application->m_RenderAPI->ClearColor(0.2f, 0.2f, 0.9f);
	Application::s_Application->m_RenderAPI->DepthTest(true);
	Application::s_Application->m_RenderAPI->Blending(true);


	// FPS camera containing view matrix
	m_Camera = new FreeCamera(Vec3(0.0f, 1.0f, 0.0f), (float)Application::s_Application->m_WindowProperties.m_Width / (float)Application::s_Application->m_WindowProperties.m_Height, 0.04f, 10.0f);

	// Framebuffer initialization data
	FrameBufferProperties frameprop;
	frameprop.m_Width = Application::s_Application->m_WindowProperties.m_Width;
	frameprop.m_Height = Application::s_Application->m_WindowProperties.m_Height;

	m_PostEffect = new PostEffect(frameprop, "res/shader/gamma.fx");
	m_Gamma = 1.0f;

	m_LightLook = Vec3(0.0, 0.0f, 15.0f);
	m_LightProjection = Mat4::Orthographic(-50.0f, 50.0f, -50.0f, 50.0f, 1.0f, 100.0f);
	m_LightView = Mat4::LookAt(
		Vec3(0.0f, 35.0f, 23.0f),
		m_LightLook,
		Vec3(0.0, 1.0, 0.0));

	m_Renderer = new Renderer3D(m_Camera);
	m_Renderer->SetPostEffect(m_PostEffect);

	//m_Scene = m_File.Read("res/sponza/sponza.das");
	//m_Scene = m_File.Read("res/knife.das");
	m_Scene = new Scene(m_Camera);

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
	m_Scene->m_LightSystem.m_Directional.m_LightCamera = m_LightProjection * m_LightView;
	Application::s_Application->m_Window->SetCursorVisible(false);
	m_Camera->ResetMouse();
	m_World = new World();
	m_WorldRenderer = std::make_shared<WorldRenderer>(m_Camera);
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
	if (Application::s_Application->m_Window->m_Keys[DODO_KEY_9])
		m_Gamma += 1.0f * elapsed;
	if (Application::s_Application->m_Window->m_Keys[DODO_KEY_8])
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

	if (Application::s_Application->m_Window->m_Keys[DODO_KEY_0])
		m_Scene->m_LightSystem.m_Directional.m_Direction = Vec3(0.0f, -1.0f, 1.0f);

	if (Application::s_Application->m_Window->m_Keys[DODO_KEY_UP])
		m_LightLook += elapsed * Vec3(1.0f, 0.0f, 0.0f) * 10.0f;
	if (Application::s_Application->m_Window->m_Keys[DODO_KEY_DOWN])
		m_LightLook -= elapsed * Vec3(1.0f, 0.0f, 0.0f) * 10.0f;
	if (Application::s_Application->m_Window->m_Keys[DODO_KEY_RIGHT])
		m_LightLook += elapsed * Vec3(0.0f, 0.0f, 1.0f) * 10.0f;
	if (Application::s_Application->m_Window->m_Keys[DODO_KEY_LEFT])
		m_LightLook -= elapsed * Vec3(0.0f, 0.0f, 1.0f) * 10.0f;
	m_LightView = Mat4::LookAt(
		Vec3(-8.0f, 35.0f, 23.0f),
		m_LightLook,
		Vec3(0.0, 1.0, 0.0));


	m_Scene->m_LightSystem.m_Directional.m_LightCamera = m_LightProjection * m_LightView;

	m_PostEffect->SetUniformValue("u_Gamma", m_Gamma);

	m_Camera->Update(elapsed);

	std::stringstream stream;
	stream << Application::s_Application->m_FramesPerSecond << " FPS, " << std::fixed << std::setprecision(2) << Application::s_Application->m_FrameTimeMs << "ms";
	std::string s = stream.str();
	Application::s_Application->m_Window->SetTitle(stream.str().c_str());
}

void GameLayer::Render()
{
	m_PostEffect->Bind();
	m_WorldRenderer->Draw(m_World);
	m_Scene->m_SkyBox->Draw(m_Camera->GetViewMatrix());
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