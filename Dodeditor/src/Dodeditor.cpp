#include "Dodeditor.h"

#include <imgui.h>
#include <imgui_internal.h>

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
	m_Rotation = Mat4::Translate(Vec3(0.0f, 0.0f, -10.0f));

	m_Projection = Mat4::Perspective(45.0f, (float)Application::s_Application->m_WindowProperties.m_Width / (float)Application::s_Application->m_WindowProperties.m_Height, 0.1f, 100.0f);

	m_Shader->Bind();
	m_Shader->SetUniformValue("u_Projection", m_Projection);
	m_Shader->SetUniformValue("u_Model", m_Rotation);

	FrameBufferProperties frameprop;
	frameprop.m_Width = Application::s_Application->m_WindowProperties.m_Width;
	frameprop.m_Height = Application::s_Application->m_WindowProperties.m_Height;

	m_FrameBuffer = new FrameBuffer(frameprop);

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
	style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	style.WindowRounding = 0.0f;

	m_Scene = new Scene();
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

	static bool s_ShowViewport = true;
	static bool s_ShowEntityHierarchy = true;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->GetWorkPos());
	ImGui::SetNextWindowSize(viewport->GetWorkSize());
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::Begin("DockSpace", 0, window_flags);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PopStyleVar();
	ImGui::PopStyleVar(2);

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("ADockSpace");
		if (ImGui::DockBuilderGetNode(dockspace_id) == NULL)
		{
			ResetDockspace(dockspace_id);
		}
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
			ImGui::MenuItem("Entity Hierarchy", "", &s_ShowEntityHierarchy);
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
	ImGui::End();

	static ImGuiWindowFlags s_ViewportWindow = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

	if (s_ShowViewport)
	{
		ImGui::Begin("Viewport", 0, s_ViewportWindow);
	
		ImGui::Text("%d fps, %gms", Application::s_Application->m_FramesPerSecond, Application::s_Application->m_FrameTimeMs);
		static float width = 0.0f; 
		static float height = 0.0f;
		if (width != ImGui::GetWindowWidth() || height != ImGui::GetWindowHeight())
		{
			width = ImGui::GetWindowWidth();
			height = ImGui::GetWindowHeight();
			m_Projection = Mat4::Perspective(45.0f, width / height, 0.1f, 100.0f);
			m_FrameBuffer->Resize((int)width, (int)height);
		}
		DrawScene();
		ImGui::Image((void*)(intptr_t)m_FrameBuffer->GetTextureHandle(), ImVec2(width, height));
		ImGui::End();
	}

	if (s_ShowEntityHierarchy)
	{
		static uint s_RenamingId = -1; // 4 294 967 295
		ImGui::Begin("Entity Hierarchy");
		if (ImGui::Button("Create New Entity"))
		{
			s_RenamingId = m_Scene->CreateEntity();
			ImGui::SetNextTreeNodeOpen(true);
		}

		if(ImGui::TreeNode("Entities"))
		{
			static char s_Renameable[32] = "Unnamed";
			for (auto ent : m_Scene->m_Entities)
			{
				if (ent.first != s_RenamingId)
				{

					if (ImGui::TreeNode(ent.second.m_Name.c_str()))
					{
						if (ImGui::Button("Add component"))
						{

						}
						ImGui::TreePop();
					}
				}
				else
				{
					ImGui::SetKeyboardFocusHere(0);
					if (ImGui::InputText(std::to_string(ent.first).c_str(), s_Renameable, IM_ARRAYSIZE(s_Renameable), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
					{
						m_Scene->RenameEntity(ent.first, s_Renameable);
						strcpy_s(s_Renameable, "Unnamed");
						s_RenamingId = -1;
					}
				}
			}
			ImGui::TreePop();
		}

		ImGui::End();
	}


	Application::s_Application->ImGuiEndFrame();
}

void GameLayer::ResetDockspace(uint dockspace_id)
{
	ImGui::DockBuilderRemoveNode(dockspace_id);
	ImGui::DockBuilderAddNode(dockspace_id, 0);
	ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(0.1f, 0.1f));

	ImGuiID dock_main_id = dockspace_id;
	ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
	ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.20f, NULL, &dock_main_id);

	ImGui::DockBuilderDockWindow("Viewport", dock_id_right);
	ImGui::DockBuilderDockWindow("Entity Hierarchy", dock_id_left);
	ImGui::DockBuilderFinish(dockspace_id);
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





Dodeditor::Dodeditor()
		: Application(PreInit())
	{}

WindowProperties Dodeditor::PreInit()
{
	WindowProperties props;
	props.m_Title = "Dodeditor";
	props.m_Width = 1080;
	props.m_Height = 720;
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