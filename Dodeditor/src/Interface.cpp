#include "Interface.h"

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

Interface::Interface(Scene* scene)
	: m_Scene(scene)
{
	InitInterface();
}

void Interface::SetChangeSceneCallback(void (*callback)(Scene*))
{
	m_ChangeSceneFunc = callback;
}

void Interface::ChangeScene(Scene* scene)
{
	
	m_Scene = scene;
	m_ChangeScene = true;
	m_SelectedEntity.clear();
	for (auto& ent : m_Scene->m_Entities)
		m_SelectedEntity.emplace(ent.first, false);
}

void Interface::InitInterface()
{
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
	ImVec4 aa = style.Colors[ImGuiCol_TabActive];
	ImVec4 aa2 = style.Colors[ImGuiCol_TitleBgActive];
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.0f, 0.5f, 0.85f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.5f, 0.85f, 1.00f);

	style.WindowRounding = 0.0f;
	style.TabRounding = 0.0f;

	m_EditorProperties.m_ViewportName = "Viewport";
	m_EditorProperties.m_HierarchyName = "Hierarchy";
	m_EditorProperties.m_InspectorName = "Inspector";

	m_EditorProperties.m_ViewportHover = false;
	m_EditorProperties.m_ViewportInput = false;

	m_EditorProperties.m_ShowViewport = true;
	m_EditorProperties.m_ShowHierarchy = true;
	m_EditorProperties.m_ShowInspector = true;

	// Hierarchy
	m_HierarchyComponents.push_back(Component("ModelComponent"));
	m_HierarchyComponents.push_back(Component("Rectangle2D"));

	// Inspector
	m_InspectorComponents = m_HierarchyComponents;
	m_CurrentInspectorName = new char[32];
	m_InspectorSelectNew = false;
}

bool Interface::BeginDraw()
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
						Scene* scene = m_File.Read(path.c_str());
						scene->m_SkyBox = m_Scene->m_SkyBox;
						delete m_Scene;
						ChangeScene(scene);
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

	if (m_EditorProperties.m_ShowHierarchy) DrawHierarchy();

	if (m_EditorProperties.m_ShowInspector) DrawInspector();

	return m_ChangeScene;
}

void Interface::EndDraw()
{
	m_ChangeScene = false;
	Application::s_Application->ImGuiEndFrame();
}

void Interface::ResetDockspace(uint dockspace_id)
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


//////////////
// Viewport //
//////////////


bool Interface::ViewportResize()
{
	if (m_ViewportWidth != ImGui::GetWindowWidth() || m_ViewportHeight != ImGui::GetWindowHeight() || m_PosX != ImGui::GetWindowPos().x || m_PosY != ImGui::GetWindowPos().y)
	{
		m_ViewportWidth = (uint)ImGui::GetWindowWidth();
		m_ViewportHeight = (uint)ImGui::GetWindowHeight();
		m_PosX = (uint)ImGui::GetWindowPos().x;
		m_PosY = (uint)ImGui::GetWindowPos().y;

		Application::s_Application->m_RenderAPI->ResizeDefaultViewport(m_ViewportWidth, m_ViewportHeight, m_PosX, m_PosY);
		return true;
	}
	return false;
}
bool Interface::BeginViewport()
{
	if (m_EditorProperties.m_ShowViewport)
	{

		static ImGuiWindowFlags s_ViewportWindow = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("Viewport", 0, s_ViewportWindow);

		ImGui::Text("%d fps, %gms", Application::s_Application->m_FramesPerSecond, Application::s_Application->m_FrameTimeMs);
		m_EditorProperties.m_ViewportHover = ImGui::IsWindowHovered();
		
		return true;
	}
	return false;
}


void Interface::EndViewport(FrameBuffer* framebuffer)
{
	if (m_EditorProperties.m_ShowViewport)
	{
		ImGui::Image((void*)(intptr_t)framebuffer->GetTextureHandle(), ImVec2((float)m_ViewportWidth, (float)m_ViewportHeight), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
	}
}

///////////////
// Hierarchy //
///////////////

void Interface::DrawHierarchy()
{
	ImGuiIO& io = ImGui::GetIO();
	static bool s_ClickHandled = false;
	static uint s_RenamingId = -1; // 4 294 967 295
	ImGui::Begin(m_EditorProperties.m_HierarchyName);

	if (ImGui::BeginPopupContextWindow("Right-Click Hierarchy", ImGuiPopupFlags_MouseButtonRight))
	{
		if (ImGui::MenuItem("Create New"))
		{
			s_RenamingId = m_Scene->CreateEntity();
			m_SelectedEntity.insert(std::make_pair(s_RenamingId, false));
			s_ClickHandled = true;
		}
		ImGui::EndPopup();
	}

	static char s_RenameableHierarchy[32] = "Unnamed";

	ImGui::ColorButton("", ImColor(226, 189, 0), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();


	if (ImGui::TreeNodeEx("Entities", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (m_Scene->m_Entities.size() > 0)
		{
			for (auto& ent : m_Scene->m_Entities)
			{
				if (ent.first != s_RenamingId || m_EditorProperties.m_ViewportInput)
				{
					ImGui::ColorButton("", ImColor(120, 50, 0), ImGuiColorEditFlags_NoTooltip);
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
						ImGui::Separator();

						if (ent.second.m_ComponentFlags != ComponentFlag_None)
						{
							ImGui::Text("Components:");
							ImGui::Indent();
							if (ent.second.m_ComponentFlags & ComponentFlag_ModelComponent)
							{
								ImGui::BulletText("ModelComponent");
							}
							if (ent.second.m_ComponentFlags & ComponentFlag_Rectangle2DComponent)
							{
								ImGui::BulletText("Rectangle2DComponent");
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
		{
			ImGui::Separator();
			ImGui::TextColored(ImVec4(0.34f, 129.0f, 0, 255), "Right click here!");
		}

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
void Interface::DrawInspector()
{
	static float translate[3] = { 0.0f, 0.0f, 0.0f };
	static float scale[3] = { 1.0f, 1.0f, 1.0f };
	static float rotate[3] = { 0.0f, 0.0f, 0.0f };
	ImGui::Begin(m_EditorProperties.m_InspectorName);
	for (auto& e : m_SelectedEntity)
	{
		if (e.second)
		{
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
			ImGui::Text("Components:");

			if (ImGui::BeginPopupContextWindow("Right-Click Inspector", ImGuiPopupFlags_MouseButtonRight))
			{
				if (ImGui::BeginMenu("Add.."))
				{
					if (ImGui::BeginMenu("Geometry"))
					{
						for (int i = 0; i < m_InspectorComponents.size(); i++)
						{
							std::string name = m_InspectorComponents[i].m_Name;
							if (ImGui::MenuItem(name.c_str()))
							{
								ent.m_ComponentFlags |= ((ComponentFlag)(i+1) << 0);
								break;
							}
						}
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Script"))
					{
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Audio"))
					{
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Other"))
					{
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				ImGui::EndPopup();
			}

			if (ent.m_ComponentFlags & ComponentFlag_ModelComponent) // ModelComponent //
			{
				if (ImGui::TreeNodeEx("ModelComponent", ImGuiTreeNodeFlags_DefaultOpen))
				{
					auto& compvar = ent.FindComponent(ModelComponent::GetIndex());
					if (compvar.index() == ModelComponent::GetIndex())
					{
						ModelComponent* model = std::get<ModelComponent::GetIndex()>(compvar);
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
							if (str._Starts_with(Application::s_Application->m_Window->GetMainWorkDirectory())) str.erase(0, Application::s_Application->m_Window->GetMainWorkDirectory().length() + 1); // In main work directory
							model = new ModelComponent(str.c_str());
							m_Scene->AddComponent(e.first, model);
						}
						ImGui::SameLine();
						ImGui::Text(model->m_Path.c_str());

						if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::Text("Translate:");
							if (ImGui::DragFloat3("##translate", translate, 0.05f))
							{
								model->m_Transformation.Move(Vec3(translate[0], translate[1], translate[2]));
							}

							ImGui::Text("Scale:");
							static bool sync = true;
							bool dragscale = false;
							dragscale = ImGui::DragFloat3("##scale", scale, 0.0001f, -100000.0f, 100000.0f, "%.4f", 1.001f);
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

			if (ent.m_ComponentFlags & ComponentFlag_Rectangle2DComponent) // Rectangle2D
			{
				if (ImGui::TreeNodeEx("Rectangle2D", ImGuiTreeNodeFlags_DefaultOpen))
				{
					auto& compvar = ent.FindComponent(Rectangle2DComponent::GetIndex());
					if (compvar.index() == Rectangle2DComponent::GetIndex())
					{
						Rectangle2DComponent* comp = std::get<Rectangle2DComponent::GetIndex()>(compvar);
						if (m_InspectorSelectNew)
						{
							memcpy(translate, (float*)&comp->m_Transformation.m_Position, 3 * sizeof(float));
							memcpy(scale, (float*)&comp->m_Transformation.m_Scale, 3 * sizeof(float));
							memcpy(rotate, (float*)&comp->m_Transformation.m_Rotation, 3 * sizeof(float));
							rotate[0] = Math::ToDegrees(rotate[0]);
							rotate[1] = Math::ToDegrees(rotate[1]);
							rotate[2] = Math::ToDegrees(rotate[2]);
							m_InspectorSelectNew = false;
						}
						ImGui::Indent();
						if (ImGui::Button("Browse")) {
							std::string str = Application::s_Application->m_Window->OpenFileSelector("Texture\0*.png;*.jpg;*.tga\0");
							if (str._Starts_with(Application::s_Application->m_Window->GetMainWorkDirectory())) str.erase(0, Application::s_Application->m_Window->GetMainWorkDirectory().length() + 1); // In main work directory
							comp = new Rectangle2DComponent(str.c_str());
							m_Scene->AddComponent(e.first, comp);
						}
						ImGui::SameLine();
						ImGui::Text(comp->m_Path.c_str());

						if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::Text("Translate:");
							if (ImGui::DragFloat3("##translate", translate, 0.05f))
							{
								comp->m_Transformation.Move(Vec3(translate[0], translate[1], translate[2]));
							}

							ImGui::Text("Scale:");
							static bool sync = true;
							bool dragscale = false;
							dragscale = ImGui::DragFloat3("##scale", scale, 0.0001f, -100000.0f, 100000.0f, "%.4f", 1.001f);
							ImGui::Checkbox("Sync", &sync);
							if (dragscale)
							{
								if (sync)
								{
									Vec3 temp = ((Vec3)scale) - comp->m_Transformation.m_Scale;

									comp->m_Transformation.m_Scale += temp.x;
									comp->m_Transformation.m_Scale += temp.y;
									comp->m_Transformation.m_Scale += temp.z;
									memcpy(scale, (float*)&comp->m_Transformation.m_Scale, sizeof(Vec3));
									comp->m_Transformation.Calculate();
								}
								else
									comp->m_Transformation.Scale(Vec3(scale[0], scale[1], scale[2]));
							}

							ImGui::Text("Rotate:");
							if (ImGui::DragFloat3("##rotate", rotate, 0.5f))
							{
								Vec3 temp = ((Vec3)rotate);
								temp = Vec3(std::fmod(temp.x, 360.0f), std::fmod(temp.y, 360.0f), std::fmod(temp.z, 360.0f));
								memcpy(rotate, (float*)&temp, sizeof(Vec3));
								comp->m_Transformation.Rotate(temp);
							}

							ImGui::TreePop();
						}
					}
					else
					{
						if (ImGui::Button("Browse")) {
							std::string str = Application::s_Application->m_Window->OpenFileSelector("Texture\0*.png;*.jpg;*.tga\0");
							if (str != "")
							{
								if (str._Starts_with(Application::s_Application->m_Window->GetMainWorkDirectory()))
									str.erase(0, Application::s_Application->m_Window->GetMainWorkDirectory().length() + 1); // In main work directory so erase global directory
								m_Scene->AddComponent(e.first, new Rectangle2DComponent(str.c_str()));
							}
						}
						ImGui::SameLine();
						ImGui::Text("...");
					}
					ImGui::TreePop();
				}
			}
			if (ent.m_ComponentFlags == 0)
			{
				ImGui::Separator();
				ImGui::TextColored(ImVec4(0.34f, 129.0f, 0, 255), "Right click here!");
			}

			if (m_InspectorSelectNew) m_InspectorSelectNew = false;
			break;
		}
	}
	ImGui::End();
}