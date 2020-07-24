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
		{ "POSITION", 3 }, 
		{ "TEXCOORD", 2 }, 
		{ "NORMAL", 3 }, 
		{ "TANGENT", 3 }
	};

	m_Rotation = Mat4::Translate(Vec3(0.0f, 0.0f, -20.0f));
	
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

	m_Scene = new Scene();
	m_Model = new Model("res/model/Bayonet.fbx");
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

	m_HierarchyComponents.push_back(Component("None"));
	m_HierarchyComponents.push_back(Component("ModelComponent"));
	m_InspectorComponents = m_HierarchyComponents;
}

GameLayer::~GameLayer()
{
	delete m_FrameBuffer;
	delete m_Camera;
	delete m_Scene;
	delete m_Model;
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


	m_Scene->UpdateCamera(m_Camera->GetCameraMatrix());
	m_Scene->Draw();

	Application::s_Application->m_RenderAPI->DefaultFrameBuffer();
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

	static ImGuiWindowFlags s_ViewportWindow = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

	if (m_EditorProperties.m_ShowViewport)
	{
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
		}
		m_EditorProperties.m_ViewportHover = ImGui::IsWindowHovered();
		DrawScene();
		ImGui::Image((void*)(intptr_t)m_FrameBuffer->GetTextureHandle(), ImVec2((float)width, (float)height), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
	}

	static char s_RenameableInspector[32] = "";

	bool s_ClickHandled = false;
	if (m_EditorProperties.m_ShowHierarchy)
	{
		static uint s_RenamingId = -1; // 4 294 967 295
		bool s_SetOpen = false;
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
		if(ImGui::TreeNode("Entities"))
		{
			if (m_Scene->m_Entities.size() > 0)
			{

				for (auto& ent : m_Scene->m_Entities)
				{
					if (ent.first != s_RenamingId || m_EditorProperties.m_ViewportInput)
					{
						ImGui::ColorButton("", ImColor(120, 50, 0), ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_NoInputs);
						ImGui::SameLine();
						bool open = ImGui::TreeNodeEx(ent.second.m_Name.c_str(), (m_SelectedEntity.at(ent.first) ? ImGuiTreeNodeFlags_Selected  : 0 ) | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);
						if (m_SelectedEntity.at(ent.first))
						{
							ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 40); // Move text to right side
							ImGui::Text("%i", ent.first);
						}
						if (ImGui::IsItemClicked())
						{
							if(!io.KeyCtrl)
								for (auto& e : m_SelectedEntity)
									e.second = false;

							m_SelectedEntity.at(ent.first) = !m_SelectedEntity.at(ent.first);
							strcpy_s(s_RenameableInspector, ent.second.m_Name.c_str());
							s_ClickHandled = true;
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

							if (ent.second.m_ComponentFlags != 0)
							{
								ImGui::Text("Components:");
								ImGui::Separator();
								ImGui::Indent();
								if (ent.second.m_ComponentFlags & FlagModelComponent)
								{
									ImGui::BulletText("ModelComponent");
								}
								ImGui::Unindent();
							}

							ImGui::TreePop();
						}
					}
					else
					{
						ImGui::SetKeyboardFocusHere(0);
						ImGui::Indent();
						if (ImGui::InputText(std::to_string(ent.first).c_str(), s_RenameableHierarchy, IM_ARRAYSIZE(s_RenameableHierarchy), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
						{
							if(s_RenameableHierarchy == '\0')
								strcpy_s(s_RenameableHierarchy, "Unnamed");

							m_Scene->RenameEntity(ent.first, s_RenameableHierarchy);
							strcpy_s(s_RenameableInspector, s_RenameableHierarchy);
							strcpy_s(s_RenameableHierarchy, "Unnamed");
							s_RenamingId = -1;
							if (!io.KeyCtrl)
								for (auto& e : m_SelectedEntity)
									e.second = false;
							m_SelectedEntity.at(ent.first) = true;
						}
						ImGui::Unindent();
					}
				}
			}
			else
				ImGui::Text("No entities here...");

			ImGui::TreePop();
			if(!s_ClickHandled && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered())
				for (auto& e : m_SelectedEntity)
					e.second = false;

			s_ClickHandled = false;
		}

		ImGui::End();
	}

	if (m_EditorProperties.m_ShowInspector)
	{
		ImGui::Begin(m_EditorProperties.m_InspectorName);
		for (auto& e : m_SelectedEntity)
		{
			if (e.second)
			{
				ImGui::Text("Name:");

				Entity& ent = m_Scene->m_Entities.at(e.first);

				if (ImGui::InputText("##label", s_RenameableInspector, IM_ARRAYSIZE(s_RenameableInspector), ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (strcmp(s_RenameableInspector, "") != 0)
					{
						m_Scene->RenameEntity(e.first, s_RenameableInspector);
					}
					else
					{
						strcpy_s(s_RenameableInspector, ent.m_Name.c_str());
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
				if (ent.m_ComponentFlags & FlagModelComponent)
				{
					if (ImGui::TreeNode("ModelComponent"))
					{
						auto comp = m_Scene->m_ModelComponent.find(e.first);
						if (comp != m_Scene->m_ModelComponent.end())
						{
							ModelComponent* model = m_Scene->m_ModelComponent.at(e.first);
							ImGui::Indent();
							if (ImGui::Button("Browse")) {
								m_Scene->AddComponent(e.first, new ModelComponent(Application::s_Application->m_Window->OpenFileDialog().c_str()));
							}
							ImGui::SameLine();
							ImGui::Text(model->m_Path.c_str());

							if(ImGui::TreeNode("Transform"))
							{
								ImGui::Text("Translate:");
								static float trans[3] = { 0.0f, 0.0f, 0.0f};
								if (ImGui::DragFloat3("##translate", trans, 0.1f))
								{
									model->m_Transformation.Move(Vec3(trans[0], trans[1], trans[2]));
								}

								ImGui::Text("Scale:");
								static bool sync = false;
								static float scale[3] = { 1.0f, 1.0f, 1.0f };
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
								static float rotate[3] = { 0.0f, 0.0f, 0.0f };
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
								m_Scene->AddComponent(e.first, new ModelComponent(Application::s_Application->m_Window->OpenFileDialog().c_str()));
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





Dodeditor::Dodeditor()
		: Application(PreInit())
	{}

WindowProperties Dodeditor::PreInit()
{
	WindowProperties props;
	props.m_Title = "Dodeditor";
	props.m_Width = 1600;
	props.m_Height = 960;
	props.m_Flags = DodoWindowFlags_IMGUI | DodoWindowFlags_IMGUIDOCKING | DodoWindowFlags_BACKFACECULL;
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