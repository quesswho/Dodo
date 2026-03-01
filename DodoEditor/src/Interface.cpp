#include "Interface.h"

#include "FileDialog.h"

#include <imgui.h>
#include <imgui_internal.h>

using namespace Dodo;
using namespace Math;


Interface::Interface(Scene* scene)
{
    m_EditorContext.scene = scene;
	InitInterface();
}

void Interface::SetChangeSceneCallback(void (*callback)(Scene*))
{
	m_ChangeSceneFunc = callback;
}

void Interface::ChangeScene(Scene* scene)
{
	
	m_EditorContext.scene = scene;
	m_EditorContext.scene->m_LightSystem.m_Directional.m_Direction = Vec3(0.4f, -1.0f, 0.4f).Normalize(); // Temporary because light direction is not stored in scene file

	m_ChangeScene = true;

	ClearSelection();
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

	ImGuiIO& io = ImGui::GetIO();
    if(FileUtils::FileExists("res/font/opensans/opensans.ttf")) {
        io.Fonts->AddFontFromFileTTF("res/font/opensans/opensans.ttf", 16);
    } else {
        DD_WARN("Could not find: res/font/opensans/opensans.ttf, using default font.");
    }

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

	// Inspector
	m_InspectorComponents = m_HierarchyComponents;

	m_EditorContext.inspectorNameBuffer = "";
	m_EditorContext.inspectorDirty = false;
}

bool Interface::BeginDraw()
{
	Application::s_Application->ImGuiNewFrame();

	static bool s_ResetDockspace = false;

	ImGuiWindowFlags dockWindow_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
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
                    std::filesystem::path path = FileDialog::OpenFile("Open Scene", "Dodo Ascii Scene File\0*.das\0");
					if (!path.empty())
					{
						Scene* scene = m_File.Read(path.string());
						scene->m_SkyBox = m_EditorContext.scene->m_SkyBox;
						delete m_EditorContext.scene;
						ChangeScene(scene);
					}
				}
				ImGui::EndMenu();
			}

			/*if (ImGui::MenuItem("Save"))
			{
				m_File.Write("test.das", m_EditorContext.scene);
			}*/

			if (ImGui::MenuItem("Save As...")) {
                std::filesystem::path path = FileDialog::SaveFile("Save As", "Dodo Ascii Scene File\0*.das\0");
				if (!path.empty())
				{
					m_File.WriteAs(path.string(), m_EditorContext.scene);
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
	if (m_ViewportWidth != ImGui::GetWindowWidth() || m_ViewportHeight != ImGui::GetWindowHeight() || m_EditorContext.viewportX != ImGui::GetWindowPos().x || m_EditorContext.viewportY != ImGui::GetWindowPos().y)
	{
		m_ViewportWidth = (uint)ImGui::GetWindowWidth();
		m_ViewportHeight = (uint)ImGui::GetWindowHeight();
		m_EditorContext.viewportX = (uint)ImGui::GetWindowPos().x;
		m_EditorContext.viewportY = (uint)ImGui::GetWindowPos().y;

		Application::s_Application->m_RenderAPI->ResizeDefaultViewport(m_EditorContext.viewportWidth, m_EditorContext.viewportHeight, m_EditorContext.viewportX, m_EditorContext.viewportY);
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
			s_RenamingId = m_EditorContext.scene->GetWorld().CreateEntity();
			SingleSelect(s_RenamingId);
			s_ClickHandled = true;
		}
		ImGui::EndPopup();
	}

	static char s_RenameableHierarchy[32] = "Unnamed";

	ImGui::ColorButton("##EntitiesIcon", ImColor(226, 189, 0), ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine();


	if (ImGui::TreeNodeEx("Entities", ImGuiTreeNodeFlags_DefaultOpen))
	{
        World& world = m_EditorContext.scene->GetWorld();
		if (!world.GetAliveEntities().empty())
		{
			for (EntityID entityId : world.GetAliveEntities())
			{
				if (entityId != s_RenamingId || m_EditorProperties.m_ViewportInput)
				{
					std::string colorButtonId = "##EntityIcon" + std::to_string(entityId);
					ImGui::ColorButton(colorButtonId.c_str(), ImColor(120, 50, 0), ImGuiColorEditFlags_NoTooltip);
					ImGui::SameLine();
					ImGui::PushID((int)entityId);
					std::string entityName = world.HasComponent<NameComponent>(entityId) ? 
						world.GetComponent<NameComponent>(entityId).m_Name : "Entity_" + std::to_string(entityId);
					bool open = ImGui::TreeNodeEx(entityName.c_str(), (m_EditorContext.selection.Contains(entityId) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);
					ImGui::PopID();
					if (m_EditorContext.selection.Contains(entityId))
					{
						ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 40); // Move text to right side
						ImGui::Text("%i", entityId);
					}
					if (ImGui::IsItemClicked())
					{
						if (io.KeyCtrl)
							ToggleSelect(entityId);
                        else
                            SingleSelect(entityId);

						if (world.HasComponent<NameComponent>(entityId)) {
							m_EditorContext.inspectorNameBuffer = world.GetComponent<NameComponent>(entityId).m_Name;
						}
						s_ClickHandled = true;

						if (m_EditorContext.selection.Contains(entityId))
                        {
							m_EditorContext.inspectorDirty = true;
							m_EditorProperties.m_ShowInspector = true;
						}
					}

					if (open)
					{
						ImGui::Separator();

						// Check what components this entity has
						bool hasComponents = world.HasAnyComponent(entityId);
						if (hasComponents)
						{
							ImGui::Text("Components:");
							ImGui::Indent();
							if (world.HasComponent<ModelComponent>(entityId))
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
					if (ImGui::InputText(std::to_string(entityId).c_str(), s_RenameableHierarchy, IM_ARRAYSIZE(s_RenameableHierarchy), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsNoBlank))
					{
						if (s_RenameableHierarchy == nullptr || s_RenameableHierarchy[0] == '\0')
							strncpy(s_RenameableHierarchy, "Unnamed", 256);

						if (world.HasComponent<NameComponent>(entityId)) {
							world.GetComponent<NameComponent>(entityId).m_Name = std::string(s_RenameableHierarchy);
						} else {
							world.AddComponent<NameComponent>(entityId, NameComponent{std::string(s_RenameableHierarchy)});
						}
						m_EditorContext.inspectorNameBuffer = std::string(s_RenameableHierarchy);
						strncpy(s_RenameableHierarchy, "Unnamed", 256);
						s_RenamingId = -1;
						if (io.KeyCtrl)
							ToggleSelect(entityId);
                        else
                            SingleSelect(entityId);
                        
						m_EditorContext.inspectorDirty = true;
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
		if (!s_ClickHandled && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered()) {
			ClearSelection();
        }

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
    World& world = m_EditorContext.scene->GetWorld();
	for (int entityId : m_EditorContext.selection.entities)
	{ 
        static char nameBuffer[64] = "";
        if (m_EditorContext.inspectorDirty) {
            strncpy(nameBuffer, m_EditorContext.inspectorNameBuffer.c_str(), sizeof(nameBuffer) - 1);
            nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        }

        if (ImGui::InputText("##label", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsNoBlank))
        {
            m_EditorContext.inspectorNameBuffer = std::string(nameBuffer);
            
            if (!m_EditorContext.inspectorNameBuffer.empty())
            {
                if (world.HasComponent<NameComponent>(entityId)) {
                    world.GetComponent<NameComponent>(entityId).m_Name = m_EditorContext.inspectorNameBuffer;
                } else {
                    world.AddComponent<NameComponent>(entityId, NameComponent{m_EditorContext.inspectorNameBuffer});
                }
            } else {
                if (world.HasComponent<NameComponent>(entityId)) {
                    m_EditorContext.inspectorNameBuffer = world.GetComponent<NameComponent>(entityId).m_Name;
                    strncpy(nameBuffer, m_EditorContext.inspectorNameBuffer.c_str(), sizeof(nameBuffer) - 1);
                }
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
                    if (ImGui::MenuItem("ModelComponent"))
                    {
                        if (!world.HasComponent<ModelComponent>(entityId)) {
                            // Add empty ModelComponent for user to configure
                            //world.AddComponent<ModelComponent>(entityId, Application::s_Application->m_AssetManager->m_MeshFactory->GetRectangleMesh());
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

        // ModelComponent
        if (world.HasComponent<ModelComponent>(entityId))
        {
            if (ImGui::TreeNodeEx("ModelComponent", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ModelComponent& model = world.GetComponent<ModelComponent>(entityId);
                if (m_EditorContext.inspectorDirty)
                {
                    memcpy(translate, (float*)&model.m_Transformation.m_Position, 3 * sizeof(float));
                    memcpy(scale, (float*)&model.m_Transformation.m_Scale, 3 * sizeof(float));
                    memcpy(rotate, (float*)&model.m_Transformation.m_Rotation, 3 * sizeof(float));
                    rotate[0] = Math::ToDegrees(rotate[0]);
                    rotate[1] = Math::ToDegrees(rotate[1]);
                    rotate[2] = Math::ToDegrees(rotate[2]);
                    m_EditorContext.inspectorDirty = false;
                }
                ImGui::Indent();
                if (ImGui::Button("Browse")) {
                    std::filesystem::path path = FileDialog::OpenFile("Open file", "Model\0*.fbx;*.obj\0");
                    if (!path.empty()) {
                        // Replace the component with new model
                        ModelID id = Application::s_Application->m_AssetManager->LoadModel(path.string());
                        world.GetComponent<ModelComponent>(entityId) = ModelComponent(id, model.m_Transformation);
                    }
                }
                ImGui::SameLine();
                ImGui::Text(Application::s_Application->m_AssetManager->GetModelPath(model.m_ModelID).c_str());

                if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Text("Translate:");
                    if (ImGui::DragFloat3("##translate", translate, 0.05f))
                    {
                        model.m_Transformation.Move(Vec3(translate[0], translate[1], translate[2]));
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
                            Vec3 temp = ((Vec3)scale) - model.m_Transformation.m_Scale;
                            model.m_Transformation.m_Scale += temp.x;
                            model.m_Transformation.m_Scale += temp.y;
                            model.m_Transformation.m_Scale += temp.z;
                            memcpy(scale, (float*)&model.m_Transformation.m_Scale, sizeof(Vec3));
                            model.m_Transformation.Calculate();
                        }
                        else
                            model.m_Transformation.Scale(Vec3(scale[0], scale[1], scale[2]));
                    }

                    ImGui::Text("Rotate:");
                    if (ImGui::DragFloat3("##rotate", rotate, 0.5f))
                    {
                        Vec3 temp = ((Vec3)rotate);
                        temp = Vec3(std::fmod(temp.x, 360.0f), std::fmod(temp.y, 360.0f), std::fmod(temp.z, 360.0f));
                        memcpy(rotate, (float*)&temp, sizeof(Vec3));
                        model.m_Transformation.Rotate(temp);
                    }

                    ImGui::TreePop();
                }
                ImGui::Unindent();
                ImGui::TreePop();
            }
        }

        // Show message if no components
        if (!world.HasAnyComponent(entityId))
        {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.34f, 129.0f, 0, 255), "Right click here!");
        }

        if (m_EditorContext.inspectorDirty) m_EditorContext.inspectorDirty = false;
        break;
	}
	ImGui::End();
}

void Interface::SingleSelect(EntityID e) {
	m_EditorContext.selection.entities.clear();
	m_EditorContext.selection.entities.push_back(e);
}

void Interface::ToggleSelect(EntityID e) {
	if (m_EditorContext.selection.Contains(e)) {
        // We want to keep the order of the entities in the selection, so we can't swap and delete the last element.
		m_EditorContext.selection.entities.erase(
			std::remove(m_EditorContext.selection.entities.begin(), m_EditorContext.selection.entities.end(), e),
			m_EditorContext.selection.entities.end()
		);
	} else {
		m_EditorContext.selection.entities.push_back(e);
	}
}

void Interface::ClearSelection() {
    m_EditorContext.selection.entities.clear();
}