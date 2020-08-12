#include "Dodeditor.h"

#include <imgui.h>
#include <imgui_internal.h>

using namespace Dodo;
using namespace Math;

static int ImGuiFilterAz09(ImGuiTextEditCallbackData* data)
{
	ImWchar c = data->EventChar;
	if (c >= 'A' && c <= 'Z') return 0;
	if (c >= 'a' && c <= 'z') return 0;
	if (c >= '0' && c <= '9') return 0;
	return 1;
}

GameLayer::GameLayer()
{
	Application::s_Application->m_RenderAPI->ClearColor(0.2f, 0.2f, 0.9f);
	Application::s_Application->m_RenderAPI->DepthTest(true);

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

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	
	style.WindowRounding = 0.0f;
	style.TabRounding = 0.0f;

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

	InitEditor();
}

void GameLayer::InitEditor()
{
	m_EditorProperties.m_ViewportName = "Viewport";
	m_EditorProperties.m_HierarchyName = "Hierarchy";
	m_EditorProperties.m_InspectorName = "Inspector";

	m_EditorProperties.m_ViewportHover = false;
	m_EditorProperties.m_ViewportInput = false;

	m_EditorProperties.m_ShowViewport = true;
	m_EditorProperties.m_ShowHierarchy = true;
	m_EditorProperties.m_ShowInspector = true;

	// Hierarchy
	m_HierarchyComponents.push_back(Component("None"));
	m_HierarchyComponents.push_back(Component("ModelComponent"));


	// Inspector
	m_InspectorComponents = m_HierarchyComponents;
	m_CurrentInspectorName = new char[32];
	m_InspectorSelectNew = false;
}

GameLayer::~GameLayer()
{
	Application::s_Application->m_Window->DefaultWorkDirectory();
	delete m_FrameBuffer;
	delete m_Camera;
	delete m_Scene;
}

void GameLayer::Update(float elapsed)
{
	if(m_EditorProperties.m_ViewportInput) m_Camera->Update(elapsed);
}


void GameLayer::Render()
{
	DrawImGui();
}

void GameLayer::DrawScene()
{
	m_FrameBuffer->Bind();


	m_Scene->UpdateCamera(m_Camera);
	m_Scene->Draw();

	Application::s_Application->m_RenderAPI->DefaultFrameBuffer();
}

void GameLayer::ChangeScene(Scene* scene)
{
	m_SelectedEntity.clear();
	for (auto ent : scene->m_Entities)
		m_SelectedEntity.emplace(ent.first, false);
}

void GameLayer::DrawImGui()
{
	Application::s_Application->ImGuiNewFrame();
	
	static bool s_ResetDockspace = false;

	ImGuiWindowFlags dockWindow_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->GetWorkPos());
	ImGui::SetNextWindowSize(viewport->GetWorkSize());
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	dockWindow_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	dockWindow_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	ImGuiIO& io = ImGui::GetIO();

	ImGui::Begin("DockSpace", 0, dockWindow_flags);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PopStyleVar();
	ImGui::PopStyleVar(2);

	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("DockSpace");
		if (ImGui::DockBuilderGetNode(dockspace_id) == NULL || s_ResetDockspace)
		{
			ResetDockspace(dockspace_id);
			s_ResetDockspace = false;
		}
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), 0);
	}

	if (ImGui::BeginMenuBar())
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImGuiCol_MenuBarBg);
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("New")) 
			{
				if (ImGui::MenuItem("Scene"))
				{

				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Open"))
			{
				if (ImGui::MenuItem("Scene"))
				{
					std::string path = Application::s_Application->m_Window->OpenFileSelector("Dodo Ascii Scene File\0*.das\0");
					if (path != "")
					{
						delete m_Scene;
						m_Scene = m_File.Read(path.c_str());
						ChangeScene(m_Scene);
					}
				}
				ImGui::EndMenu();
			}

			/*if (ImGui::MenuItem("Save"))
			{
				Application::s_Application->m_Window->DefaultWorkDirectory();
				
				m_File.Write("test.das", m_Scene);
			}*/

			if (ImGui::MenuItem("Save As..."))
			{
				Application::s_Application->m_Window->DefaultWorkDirectory();
				std::string path = Application::s_Application->m_Window->OpenFileSaver("Dodo Ascii Scene File\0*.das\0", ".das");
				if (path != "")
				{
					m_File.Write(path.c_str(), m_Scene);
				}

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
			ImGui::MenuItem(m_EditorProperties.m_ViewportName, "", &m_EditorProperties.m_ShowViewport);
			ImGui::MenuItem(m_EditorProperties.m_HierarchyName, "", &m_EditorProperties.m_ShowHierarchy);
			ImGui::MenuItem(m_EditorProperties.m_InspectorName, "", &m_EditorProperties.m_ShowInspector);
			ImGui::Separator();
			if (ImGui::Button("Reset DockSpace"))
			{
				s_ResetDockspace = true;
			}
			ImGui::EndMenu();
		}
		ImGui::PopStyleColor();
		ImGui::EndMenuBar();
	}
	ImGui::End();

	if (m_EditorProperties.m_ShowViewport) DrawViewport();

	if (m_EditorProperties.m_ShowHierarchy) DrawHierarchy();

	if (m_EditorProperties.m_ShowInspector) DrawInspector();

	Application::s_Application->ImGuiEndFrame();
}

void GameLayer::ResetDockspace(uint dockspace_id)
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::DockBuilderRemoveNode(dockspace_id);
	ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

	ImGuiID dock_main_id = dockspace_id;
	ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
	ImGuiID dock_id_left_down = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.20f, NULL, &dock_id_left);

	ImGui::DockBuilderDockWindow(m_EditorProperties.m_ViewportName, dock_main_id);
	ImGui::DockBuilderDockWindow(m_EditorProperties.m_HierarchyName, dock_id_left);
	ImGui::DockBuilderDockWindow(m_EditorProperties.m_InspectorName, dock_id_left_down);
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
					break;
				case DODO_KEY_Z:
					if (m_EditorProperties.m_ViewportHover && !m_EditorProperties.m_ViewportInput)
					{
						m_EditorProperties.m_ViewportInput = true;
						Application::s_Application->m_Window->SetCursorVisible(false);
						m_Camera->ResetMouse();
					}
					else if (m_EditorProperties.m_ViewportInput)
					{
						m_EditorProperties.m_ViewportInput = false;
						Application::s_Application->m_Window->SetCursorVisible(true);
					}
					break;
			}
			break;
		case EventType::MOUSE_PRESSED:
			break;
		case EventType::MOUSE_POSITION:
			if (m_EditorProperties.m_ViewportInput) m_Camera->UpdateRotation();
	}
}

//////////////
// Viewport //
//////////////

void GameLayer::DrawViewport()
{
	static ImGuiWindowFlags s_ViewportWindow = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Viewport", 0, s_ViewportWindow);

	ImGui::Text("%d fps, %gms", Application::s_Application->m_FramesPerSecond, Application::s_Application->m_FrameTimeMs);
	static uint width = 0;
	static uint height = 0;
	static uint posX = 0;
	static uint posY = 0;
	if (width != ImGui::GetWindowWidth() || height != ImGui::GetWindowHeight() || posX != ImGui::GetWindowPos().x || posY != ImGui::GetWindowPos().y)
	{
		width = (uint)ImGui::GetWindowWidth();
		height = (uint)ImGui::GetWindowHeight();
		posX = (uint)ImGui::GetWindowPos().x;
		posY = (uint)ImGui::GetWindowPos().y;

		m_Camera->Resize(width, height);
		m_FrameBuffer->Resize(width, height);
		Application::s_Application->m_RenderAPI->ResizeDefaultViewport(width, height, posX, posY);
		m_Scene->m_SkyBox->m_Projection = m_Camera->GetProjectionMatrix();
	}
	m_EditorProperties.m_ViewportHover = ImGui::IsWindowHovered();
	DrawScene();
	ImGui::Image((void*)(intptr_t)m_FrameBuffer->GetTextureHandle(), ImVec2((float)width, (float)height), ImVec2(0, 1), ImVec2(1, 0));
	ImGui::End();
}

///////////////
// Hierarchy //
///////////////

void GameLayer::DrawHierarchy()
{
	ImGuiIO& io = ImGui::GetIO();
	static bool s_ClickHandled = false;
	static uint s_RenamingId = -1; // 4 294 967 295
	static bool s_SetOpen = false;
	ImGui::Begin(m_EditorProperties.m_HierarchyName);
	if (ImGui::Button("Create New Entity"))
	{
		s_RenamingId = m_Scene->CreateEntity();
		m_SelectedEntity.insert(std::make_pair(s_RenamingId, false));
		s_ClickHandled = true;
		s_SetOpen = true;
	}


	static char s_RenameableHierarchy[32] = "Unnamed";

	ImGui::ColorButton("", ImColor(226, 189, 0), ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_NoInputs);
	ImGui::SameLine();
	if (s_SetOpen)
	{
		s_SetOpen = false;
		ImGui::SetNextTreeNodeOpen(true);
	}
	if (ImGui::TreeNode("Entities"))
	{
		if (m_Scene->m_Entities.size() > 0)
		{

			for (auto& ent : m_Scene->m_Entities)
			{
				if (ent.first != s_RenamingId || m_EditorProperties.m_ViewportInput)
				{
					ImGui::ColorButton("", ImColor(120, 50, 0), ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_NoInputs);
					ImGui::SameLine();
					ImGui::PushID((int)ent.first);
					bool open = ImGui::TreeNodeEx(ent.second.m_Name.c_str(), (m_SelectedEntity.at(ent.first) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);
					ImGui::PopID();
					if (m_SelectedEntity.at(ent.first))
					{
						ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 40); // Move text to right side
						ImGui::Text("%i", ent.first);
					}
					if (ImGui::IsItemClicked())
					{
						if (!io.KeyCtrl)
							for (auto& e : m_SelectedEntity)
								e.second = false;

						m_SelectedEntity.at(ent.first) = !m_SelectedEntity.at(ent.first);
						strcpy(m_CurrentInspectorName, ent.second.m_Name.c_str());
						s_ClickHandled = true;
						if (m_SelectedEntity.at(ent.first))
						{
							m_InspectorSelectNew = true;
							m_EditorProperties.m_ShowInspector = true;
						}
					}

					if (open)
					{
						static uint s_Selected = 1;
						const char* selectedName = m_HierarchyComponents[s_Selected].m_Name;
						ImGui::Indent();
						if (ImGui::BeginCombo("###label", selectedName, 0))
						{
							for (int i = 0; i < m_HierarchyComponents.size(); i++)
							{
								auto& comp = m_HierarchyComponents[i];
								std::string name = comp.m_Name;
								if (ImGui::Selectable(name.c_str(), comp.m_Selected))
								{
									s_Selected = i;
									break;
								}
							}
							ImGui::EndCombo();
						}


						ImGui::Indent();
						if (ImGui::Button("Add component"))
						{
							if (s_Selected != 0)
							{

								for (auto& e : m_SelectedEntity)
									e.second = false;
								m_SelectedEntity.at(ent.first) = true;

								ent.second.m_ComponentFlags |= 1 << (s_Selected - 1);
							}
						}
						ImGui::Unindent();
						ImGui::Separator();

						if (ent.second.m_ComponentFlags != ComponentFlag_None)
						{
							ImGui::Text("Components:");
							ImGui::Indent();
							if (ent.second.m_ComponentFlags & ComponentFlag_ModelComponent)
							{
								ImGui::BulletText("ModelComponent");
							}
							ImGui::Unindent();
						}
						ImGui::Unindent();
						ImGui::Separator();
						ImGui::TreePop();
					}
				}
				else
				{
					ImGui::SetKeyboardFocusHere(0);
					ImGui::Indent();
					if (ImGui::InputText(std::to_string(ent.first).c_str(), s_RenameableHierarchy, IM_ARRAYSIZE(s_RenameableHierarchy), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackCharFilter, ImGuiFilterAz09))
					{
						if (s_RenameableHierarchy == '\0')
							strcpy_s(s_RenameableHierarchy, "Unnamed");

						m_Scene->RenameEntity(ent.first, s_RenameableHierarchy);
						strcpy(m_CurrentInspectorName, s_RenameableHierarchy);
						strcpy_s(s_RenameableHierarchy, "Unnamed");
						s_RenamingId = -1;
						if (!io.KeyCtrl)
							for (auto& e : m_SelectedEntity)
								e.second = false;
						m_SelectedEntity.at(ent.first) = true;
						m_InspectorSelectNew = true;
					}
					ImGui::Unindent();
				}
			}
		}
		else
			ImGui::Text("No entities here...");

		ImGui::TreePop();
		if (!s_ClickHandled && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered())
			for (auto& e : m_SelectedEntity)
				e.second = false;

		s_ClickHandled = false;
	}

	ImGui::End();
}

///////////////
// Inspector //
///////////////
void GameLayer::DrawInspector()
{
	static float translate[3] = { 0.0f, 0.0f, 0.0f };
	static float scale[3] = { 1.0f, 1.0f, 1.0f };
	static float rotate[3] = { 0.0f, 0.0f, 0.0f };
	ImGui::Begin(m_EditorProperties.m_InspectorName);
	for (auto& e : m_SelectedEntity)
	{
		if (e.second)
		{
			ImGui::Text("Name:");

			Entity& ent = m_Scene->m_Entities.at(e.first);

			if (ImGui::InputText("##label", m_CurrentInspectorName, 32, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCharFilter, ImGuiFilterAz09))
			{
				if (strcmp(m_CurrentInspectorName, "") != 0)
				{
					m_Scene->RenameEntity(e.first, m_CurrentInspectorName);
				}
				else
				{
					strcpy(m_CurrentInspectorName, ent.m_Name.c_str());
				}
			}

			ImGui::Separator();

			static uint s_Selected = 1;
			const char* selectedName = m_InspectorComponents[s_Selected].m_Name;
			if (ImGui::BeginCombo("###label", selectedName, 0))
			{
				for (int i = 0; i < m_InspectorComponents.size(); i++)
				{
					auto& comp = m_InspectorComponents[i];
					std::string name = comp.m_Name;
					if (ImGui::Selectable(name.c_str(), comp.m_Selected))
					{
						s_Selected = i;
						break;
					}
				}
				ImGui::EndCombo();
			}
			if (m_InspectorSelectNew)
				s_Selected = m_InspectorComponents.size() > 0 ? 1 : 0;

			ImGui::Indent();
			if (ImGui::Button("Add component"))
			{
				if (s_Selected != 0)
				{
					ent.m_ComponentFlags |= 1 << (s_Selected - 1);
				}
			}
			ImGui::Unindent();
			ImGui::Separator();

			ImGui::Text("Components:");
			if (ent.m_ComponentFlags & ComponentFlag_ModelComponent)
			{
				if (ImGui::TreeNode("ModelComponent"))
				{
					auto comp = m_Scene->m_ModelComponent.find(e.first);
					if (comp != m_Scene->m_ModelComponent.end())
					{
						ModelComponent* model = m_Scene->m_ModelComponent.at(e.first);
						if (m_InspectorSelectNew)
						{
							memcpy(translate, (float*)&model->m_Transformation.m_Position, 3 * sizeof(float));
							memcpy(scale, (float*)&model->m_Transformation.m_Scale, 3 * sizeof(float));
							memcpy(rotate, (float*)&model->m_Transformation.m_Rotation, 3 * sizeof(float));
							rotate[0] = Math::ToDegrees(rotate[0]);
							rotate[1] = Math::ToDegrees(rotate[1]);
							rotate[2] = Math::ToDegrees(rotate[2]);
							m_InspectorSelectNew = false;
						}
						ImGui::Indent();
						if (ImGui::Button("Browse")) {
							std::string str = Application::s_Application->m_Window->OpenFileSelector("Model\0*.fbx;*.obj\0");
							if (str._Starts_with(Application::s_Application->m_Window->GetMainWorkDirectory())) str.erase(Application::s_Application->m_Window->GetMainWorkDirectory().length()); // In main work directory
							m_Scene->AddComponent(e.first, new ModelComponent(str.c_str()));
						}
						ImGui::SameLine();
						ImGui::Text(model->m_Path.c_str());

						if (ImGui::TreeNode("Transform"))
						{
							ImGui::Text("Translate:");
							if (ImGui::DragFloat3("##translate", translate, 0.05f))
							{
								model->m_Transformation.Move(Vec3(translate[0], translate[1], translate[2]));
							}

							ImGui::Text("Scale:");
							static bool sync = true;
							bool dragscale = false;
							dragscale = ImGui::DragFloat3("##scale", scale, 0.002f, -100000.0f, 100000.0f, "%.3f", 1.0f);
							ImGui::Checkbox("Sync", &sync);
							if (dragscale)
							{
								if (sync)
								{
									Vec3 temp = ((Vec3)scale) - model->m_Transformation.m_Scale;

									model->m_Transformation.m_Scale += temp.x;
									model->m_Transformation.m_Scale += temp.y;
									model->m_Transformation.m_Scale += temp.z;
									memcpy(scale, (float*)&model->m_Transformation.m_Scale, sizeof(Vec3));
									model->m_Transformation.Calculate();
								}
								else
									model->m_Transformation.Scale(Vec3(scale[0], scale[1], scale[2]));
							}

							ImGui::Text("Rotate:");
							if (ImGui::DragFloat3("##rotate", rotate, 0.5f))
							{
								Vec3 temp = ((Vec3)rotate);
								temp = Vec3(std::fmod(temp.x, 360.0f), std::fmod(temp.y, 360.0f), std::fmod(temp.z, 360.0f));
								memcpy(rotate, (float*)&temp, sizeof(Vec3));
								model->m_Transformation.Rotate(temp);
							}

							ImGui::TreePop();
						}
					}
					else
					{
						if (ImGui::Button("Browse")) {
							std::string str = Application::s_Application->m_Window->OpenFileSelector("Model\0*.fbx;*.obj\0");
							if (str != "")
							{
								if (str._Starts_with(Application::s_Application->m_Window->GetMainWorkDirectory())) 
									str.erase(0, Application::s_Application->m_Window->GetMainWorkDirectory().length() + 1); // In main work directory so erase global directory
								m_Scene->AddComponent(e.first, new ModelComponent(str.c_str()));
							}
						}
						ImGui::SameLine();
						ImGui::Text("...");
					}
					ImGui::TreePop();
				}
			}
			break;
		}
	}
	ImGui::End();
}


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