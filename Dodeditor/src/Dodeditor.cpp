#include "Dodeditor.h"

#include <imgui_impl_win32.h>
#include <imgui_impl_opengl3.h>

using namespace Dodo;
using namespace Math;

GameLayer::GameLayer()
{
	Application::s_Application->m_RenderAPI->ClearColor(0.2f, 0.2f, 0.9f);
	Application::s_Application->m_RenderAPI->DepthTest(true);

	BufferProperties bufferprop = {
		{ "POSITION", 3 } //vertices
	};
	

	float verts[] = {
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f
	};

	uint indices[] = {
		1,0,2,
		2,0,3,

		6,7,4,
		4,5,6,

		0,1,4,
		4,1,5,

		6,2,3,
		6,3,7,

		1,2,5,
		5,2,6,

		7,3,0,
		7,0,4
	};

	m_VBuffer = new VertexBuffer(verts, sizeof(verts), bufferprop);
	m_IBuffer = new IndexBuffer(indices, _countof(indices));
	m_VAO = new ArrayBuffer(m_VBuffer, m_IBuffer);

	m_Shader = new Shader("Test", "res/shader/Shader.glsl", bufferprop);
	m_Rotation = Mat4(1.0f);

	m_Shader->SetUniformValue("u_Model", m_Rotation);

	FrameBufferProperties frameprop;
	frameprop.m_Width = Application::s_Application->m_WindowProperties.m_Width;
	frameprop.m_Height = Application::s_Application->m_WindowProperties.m_Height;

	m_FrameBuffer = new FrameBuffer(frameprop);
}
GameLayer::~GameLayer()
{
	delete m_VBuffer;
	delete m_IBuffer;
	delete m_VAO;
	delete m_Shader;
	delete m_FrameBuffer;
}

void GameLayer::Update(float elapsed)
{
	m_Rotation *= Mat4::Rotate(45.0f * elapsed, Math::Vec3(1.0f, 1.0f, 0.0f));
}


void GameLayer::Render()
{
	DrawImGui();
}

void GameLayer::DrawScene()
{
	m_FrameBuffer->Bind();

	m_VAO->Bind();
	m_Shader->Bind();
	m_Shader->SetUniformValue("u_Model", m_Rotation);
	Application::s_Application->m_RenderAPI->DrawIndices(m_VAO->GetCount());

	Application::s_Application->m_RenderAPI->DefaultFrameBuffer();
}

void GameLayer::DrawImGui()
{
	Application::s_Application->ImGuiNewFrame();

	//static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	static bool s_ShowViewport = false;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->GetWorkPos());
	ImGui::SetNextWindowSize(viewport->GetWorkSize());
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	window_flags |= ImGuiWindowFlags_NoBackground;

	ImGui::Begin("DockSpace Demo", 0, window_flags);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PopStyleVar();
	ImGui::PopStyleVar(2);

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), 0);
	}

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("New")) 
			{
				ImGui::MenuItem("Project");
				ImGui::MenuItem("Scene");
				ImGui::MenuItem("Entity");
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Open"))
			{
				ImGui::MenuItem("Project");
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Save"))
			{
				ImGui::MenuItem("Project");
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Import/Export"))
			{
				ImGui::MenuItem("Model");
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window"))
		{
			ImGui::MenuItem("Viewport", "", &s_ShowViewport);
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
	ImGui::End();

	if (s_ShowViewport)
	{
		ImGui::Begin("Viewport");
		ImGui::Text("%d fps, %gms", Application::s_Application->m_FramesPerSecond, Application::s_Application->m_FrameTimeMs);
		const uint width = ImGui::GetWindowWidth();
		const uint height = ImGui::GetWindowHeight();
		DrawScene();
		ImGui::Image((void*)(intptr_t)m_FrameBuffer->GetTextureHandle(), ImVec2(Application::s_Application->m_WindowProperties.m_Width, Application::s_Application->m_WindowProperties.m_Height));
		ImGui::End();
	}
	Application::s_Application->ImGuiEndFrame();
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
				case DODO_KEY_F11:
					Application::s_Application->m_Window->FullScreen();
			}
			
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
		props.m_Title = "Dodeditor";
		props.m_Width = 1080;
		props.m_Height = 720;
		props.m_Fullscreen = false;
		props.m_Vsync = false;
		props.m_ImGUI = true;
		props.m_ImGUIDocking = true;
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