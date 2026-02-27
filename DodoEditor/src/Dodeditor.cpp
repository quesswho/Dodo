#include "Dodeditor.h"

using namespace Dodo;
using namespace Math;

GameLayer::GameLayer()
{
	Application::s_Application->m_RenderAPI->ClearColor(0.2f, 0.2f, 0.9f);
	Application::s_Application->m_RenderAPI->DepthTest(true);
	Application::s_Application->m_RenderAPI->Blending(true);

	BufferProperties bufferprop = {
		{ "POSITION", 3 }, 
		{ "TEXCOORD", 2 }, 
		{ "NORMAL", 3 }, 
		{ "TANGENT", 3 }
	};
	
	m_Camera = new FreeCamera(Vec3(0.0f, 0.0f, 20.0f), (float)Application::s_Application->m_WindowProperties.m_Width / (float)Application::s_Application->m_WindowProperties.m_Height, 0.04f, 10.0f);

	FrameBufferProperties frameprop;
	frameprop.m_Width = Application::s_Application->m_WindowProperties.m_Width;
	frameprop.m_Height = Application::s_Application->m_WindowProperties.m_Height;

	m_FrameBuffer = new FrameBuffer(frameprop);

	m_Renderer = new EditorRenderer(m_Camera);
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

	m_Interface = new Interface(m_Scene);
}

void GameLayer::SetScene(Scene* scene)
{
	m_Scene = scene;
}

GameLayer::~GameLayer()
{
	Application::s_Application->m_Window->DefaultWorkDirectory();
	delete m_FrameBuffer;
	delete m_Camera;
	delete m_Scene;
	delete m_Interface;
}

void GameLayer::Update(float elapsed)
{
	if(m_Interface->m_EditorProperties.m_ViewportInput) m_Camera->Update(elapsed);
}


void GameLayer::Render()
{
	if (m_Interface->BeginDraw())
	{
		m_Scene = m_Interface->m_Scene; // Maybe work out something better when changing scene
	}

	m_Interface->BeginViewport();
	if (m_Interface->ViewportResize())
	{
		m_Camera->Resize(m_Interface->m_ViewportWidth, m_Interface->m_ViewportHeight);
		m_FrameBuffer->Resize(m_Interface->m_ViewportWidth, m_Interface->m_ViewportHeight);
		
		if (m_Scene->m_SkyBox != nullptr) m_Scene->m_SkyBox->m_Projection = m_Camera->GetProjectionMatrix();
	}
	DrawScene();
	m_Interface->EndViewport(m_FrameBuffer);
	m_Interface->EndDraw();
}

void GameLayer::DrawScene()
{
	m_FrameBuffer->Bind();

	m_Renderer->UpdateCamera(m_Camera);
	m_Renderer->DrawScene(m_Scene);
	//m_Scene->UpdateCamera(m_Camera);
	//m_Scene->Draw();

	Application::s_Application->m_RenderAPI->DefaultFrameBuffer();
}

void GameLayer::OnEvent(const Event& event)
{
	switch (event.GetType())
	{
		case EventType::KEY_PRESSED:
			switch (static_cast<const KeyPressEvent&>(event).m_Key)
			{
				case DODO_KEY_ESCAPE:
					Application::s_Application->Shutdown();
					break;
				case DODO_KEY_Z:
					if (m_Interface->m_EditorProperties.m_ViewportHover && !m_Interface->m_EditorProperties.m_ViewportInput)
					{
						m_Interface->m_EditorProperties.m_ViewportInput = true;
						Application::s_Application->m_Window->SetCursorVisible(false);
						m_Camera->ResetMouse();
					}
					else if (m_Interface->m_EditorProperties.m_ViewportInput)
					{
						m_Interface->m_EditorProperties.m_ViewportInput = false;
						Application::s_Application->m_Window->SetCursorVisible(true);
					}
					break;
				case DODO_KEY_DELETE:
					if (!Application::s_Application->GetInput().IsKeyPressed(DODO_KEY_LEFT_CONTROL)) {
						break;
					}
					std::vector<decltype(m_Interface->m_SelectedEntity)::key_type> toDelete;
					for (auto&& e : m_Interface->m_SelectedEntity)
					{
						if (e.second) 
						{
							m_Scene->DeleteEntity(e.first);
							toDelete.emplace_back(e.first); // Cant erase item while looping over map. Therefore erase later
						}
					}

					for (auto&& key : toDelete)
						m_Interface->m_SelectedEntity.erase(key);
					break;
			}
			break;
		case EventType::MOUSE_PRESSED:
			break;
		case EventType::MOUSE_POSITION:
			if (m_Interface->m_EditorProperties.m_ViewportInput) m_Camera->UpdateRotation();
			break;
	}
}

// Entry //

Dodeditor::Dodeditor()
		: Application(PreInit())
	{}

WindowProperties Dodeditor::PreInit()
{
	WindowProperties props;
	props.m_Title = "Dodeditor";
	props.m_Width = 1600;
	props.m_Height = 960;
	props.m_Flags = DodoWindowFlags_IMGUI | DodoWindowFlags_IMGUIDOCKING;
	return props;
}

void Dodeditor::Init()
{
	PushLayer(new GameLayer());
}


int main() {
	Dodeditor* sandBox = new Dodeditor();
	sandBox->Run();
	delete sandBox;
}