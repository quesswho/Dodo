#include "Interface.h"

#include "Data/EditorSceneFile.h"
#include "FileDialog.h"

#include <imgui.h>
#include <imgui_internal.h>

using namespace Dodo;
using namespace Math;

Interface::Interface(EditorScene* scene)
{
    m_EditorState.scene = scene;
    InitInterface();
}

void Interface::SetActiveProject(std::unique_ptr<Dodo::Project> project)
{
    m_Project = std::move(project);
    m_AssetBrowserState.projectRoot = m_Project->GetAssetsDir();
    m_AssetBrowserState.currentDir = m_Project->GetAssetsDir();
}

void Interface::ChangeScene(EditorScene* scene)
{
    m_EditorState.scene = scene;
    m_EditorState.scene->m_LightSystem.m_Directional.m_Direction =
        Vec3(0.4f, -1.0f, 0.4f).Normalize(); // Temporary because light direction is not stored in scene file

    m_ChangeScene = true;

    m_EditorState.selection.Clear();
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
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.0f, 0.5f, 0.85f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.5f, 0.85f, 1.00f);

    style.WindowRounding = 0.0f;
    style.TabRounding = 0.0f;

    ImGuiIO& io = ImGui::GetIO();
    if (FileUtils::FileExists("res/font/opensans/opensans.ttf")) {
        io.Fonts->AddFontFromFileTTF("res/font/opensans/opensans.ttf", 16);
    } else {
        DD_WARN("Could not find: res/font/opensans/opensans.ttf, using default font.");
    }

    // Viewport
    m_EditorProperties.m_ViewportHover = false;
    m_EditorProperties.m_ViewportInput = false;

    m_ViewportState.name = "Viewport";
    m_ViewportState.visible = true;

    // Hierarchy
    m_HierarchyState.name = "Hierarchy";
    m_HierarchyState.visible = true;

    // Inspector
    m_InspectorState.name = "Inspector";
    m_InspectorState.visible = false;
    m_InspectorState.dirty = false;

    // Asset Browser
    m_AssetBrowserState.name = "Asset Browser";
    m_AssetBrowserState.visible = true;

    m_Icons.Load(*Application::s_Application->m_RenderAPI);
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
    dockWindow_flags |=
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    dockWindow_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGuiIO& io = ImGui::GetIO();

    ImGui::Begin("DockSpace", nullptr, dockWindow_flags);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("DockSpace");
        if (ImGui::DockBuilderGetNode(dockspace_id) == NULL || s_ResetDockspace) {
            ResetDockspace(dockspace_id);
            s_ResetDockspace = false;
        }
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), 0);
    }

    if (ImGui::BeginMenuBar()) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGuiCol_MenuBarBg);
        if (ImGui::BeginMenu("File")) {
            if (ImGui::BeginMenu("New")) {
                if (ImGui::MenuItem("Project")) {
                    memset(m_NewProjectNameBuf, 0, sizeof(m_NewProjectNameBuf));
                    strncpy(m_NewProjectNameBuf, "MyGame", sizeof(m_NewProjectNameBuf) - 1);
                    m_NewProjectDir.clear();
                    m_NewProjectError.clear();
                    ImGui::OpenPopup("New Project##modal");
                }
                if (ImGui::MenuItem("Scene")) {
                    EditorScene* scene = new EditorScene();

                    // TODO: We are storing skybox is two different places. Needs some architectural changes
                    scene->m_SkyBox = m_EditorState.scene->m_SkyBox;
                    m_EditorState.scene->m_SkyBox = nullptr;
                    delete m_EditorState.scene;
                    ChangeScene(scene);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Open")) {
                if (ImGui::MenuItem("Project")) {
                    std::filesystem::path path =
                        FileDialog::OpenFile("Open Project", "Dodo Project File\0*.dproject\0");
                    if (!path.empty()) {
                        auto project = Dodo::Project::Open(path);
                        if (project)
                            SetActiveProject(std::move(project));
                    }
                }
                if (ImGui::MenuItem("Scene")) {
                    std::filesystem::path path = FileDialog::OpenFile("Open Scene", "Dodo Ascii Scene File\0*.das\0");
                    if (!path.empty()) {
                        EditorScene* scene = fileReader.Read(path.string());
                        // Move skybox ownership to the new scene.
                        scene->m_SkyBox = m_EditorState.scene->m_SkyBox;
                        m_EditorState.scene->m_SkyBox = nullptr;
                        delete m_EditorState.scene;
                        ChangeScene(scene);
                    }
                }
                ImGui::EndMenu();
            }

            /*if (ImGui::MenuItem("Save"))
            {
                m_File.Write(m_EditorState.scene);
            }*/

            if (ImGui::MenuItem("Save As...")) {
                std::filesystem::path path = FileDialog::SaveFile("Save As", "Dodo Ascii Scene File\0*.das\0");
                if (!path.empty()) {
                    fileReader.WriteAs(path.string(), m_EditorState.scene);
                }
            }

            if (ImGui::BeginMenu("Import/Export")) {
                ImGui::MenuItem("Model");
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            ImGui::MenuItem(m_ViewportState.name.c_str(), "", &m_ViewportState.visible);
            ImGui::MenuItem(m_HierarchyState.name.c_str(), "", &m_HierarchyState.visible);
            ImGui::MenuItem(m_InspectorState.name.c_str(), "", &m_InspectorState.visible);
            ImGui::MenuItem(m_AssetBrowserState.name.c_str(), "", &m_AssetBrowserState.visible);
            ImGui::Separator();
            if (ImGui::Button("Reset DockSpace")) {
                s_ResetDockspace = true;
            }
            ImGui::EndMenu();
        }
        ImGui::PopStyleColor();
        ImGui::EndMenuBar();
    }

    DrawNewProjectModal();

    ImGui::End();

    m_HierarchyPanel.Draw(m_EditorState, m_InspectorState, m_HierarchyState, m_Icons.Get());
    m_InspectorPanel.Draw(m_EditorState, m_InspectorState);
    m_AssetBrowserPanel.Draw(m_AssetBrowserState);

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

    ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);
    ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.20f, nullptr, &dock_main_id);

    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);

    ImGuiID dock_left_bottom;
    ImGuiID dock_left_top = ImGui::DockBuilderSplitNode(dock_left, ImGuiDir_Up, 0.50f, nullptr, &dock_left_bottom);

    ImGui::DockBuilderDockWindow(m_ViewportState.name.c_str(), dock_main_id);
    ImGui::DockBuilderDockWindow(m_HierarchyState.name.c_str(), dock_left_top);
    ImGui::DockBuilderDockWindow(m_InspectorState.name.c_str(), dock_right);
    ImGui::DockBuilderDockWindow(m_AssetBrowserState.name.c_str(), dock_bottom);

    ImGui::DockBuilderFinish(dockspace_id);
}

//////////////////////
// New Project Modal //
//////////////////////

void Interface::DrawNewProjectModal()
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSizeConstraints(ImVec2(500.0f, 0.0f), ImVec2(500.0f, FLT_MAX));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    bool opened = ImGui::BeginPopupModal("New Project##modal", nullptr,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
    ImGui::PopStyleVar();

    if (!opened)
        return;

    // Blue accent header band
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.45f, 0.78f, 1.0f));
    if (ImGui::BeginChild("##NPHdr", ImVec2(0.0f, 52.0f), false, ImGuiWindowFlags_NoScrollbar)) {
        ImGui::SetCursorPos(ImVec2(18.0f, 16.0f));
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Create New Project");
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    const float pad = 18.0f;
    const float contentW = 500.0f - pad * 2.0f;
    ImGuiStyle& sty = ImGui::GetStyle();

    // Project name
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 14.0f);
    ImGui::SetCursorPosX(pad);
    ImGui::Text("Project Name");
    ImGui::SetCursorPosX(pad);
    ImGui::SetNextItemWidth(contentW);
    ImGui::InputText("##NPName", m_NewProjectNameBuf, sizeof(m_NewProjectNameBuf));

    // Location
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
    ImGui::SetCursorPosX(pad);
    ImGui::Text("Location");
    ImGui::SetCursorPosX(pad);
    const float browseW = 80.0f;
    char locBuf[1024] = {};
    snprintf(locBuf, sizeof(locBuf), "%s",
             m_NewProjectDir.empty() ? "(not set)" : m_NewProjectDir.generic_string().c_str());
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
    ImGui::SetNextItemWidth(contentW - browseW - sty.ItemSpacing.x);
    ImGui::InputText("##NPLoc", locBuf, sizeof(locBuf), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    if (ImGui::Button("Browse", ImVec2(browseW, 0.0f))) {
        std::filesystem::path chosen = FileDialog::SelectDirectory("Select Project Location");
        if (!chosen.empty())
            m_NewProjectDir = chosen;
    }

    // Path preview
    if (!m_NewProjectDir.empty() && m_NewProjectNameBuf[0] != '\0') {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
        ImGui::SetCursorPosX(pad);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.50f, 0.50f, 1.0f));
        auto preview = m_NewProjectDir / m_NewProjectNameBuf;
        ImGui::TextWrapped("Will be created at: %s", preview.generic_string().c_str());
        ImGui::PopStyleColor();
    }

    // Error message
    if (!m_NewProjectError.empty()) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);
        ImGui::SetCursorPosX(pad);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.30f, 0.30f, 1.0f));
        ImGui::TextWrapped("%s", m_NewProjectError.c_str());
        ImGui::PopStyleColor();
    }

    // Separator
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 12.0f);
    ImGui::SetCursorPosX(pad);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::Dummy(ImVec2(contentW, 1.0f));
    ImGui::GetWindowDrawList()->AddLine(
        ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
        IM_COL32(60, 60, 60, 255));
    ImGui::PopStyleVar();

    // Buttons (right-aligned)
    const float btnW = 88.0f;
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(pad + contentW - (btnW * 2.0f + sty.ItemSpacing.x));

    if (ImGui::Button("Cancel", ImVec2(btnW, 0.0f))) {
        m_NewProjectError.clear();
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.0f, 0.45f, 0.78f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.55f, 0.88f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.0f, 0.35f, 0.68f, 1.0f));
    if (ImGui::Button("Create", ImVec2(btnW, 0.0f))) {
        if (m_NewProjectNameBuf[0] == '\0') {
            m_NewProjectError = "Project name cannot be empty.";
        } else if (m_NewProjectDir.empty()) {
            m_NewProjectError = "Please select a location.";
        } else {
            auto proj = Dodo::Project::New(m_NewProjectDir, m_NewProjectNameBuf);
            if (proj) {
                SetActiveProject(std::move(proj));
                m_NewProjectError.clear();
                ImGui::CloseCurrentPopup();
            } else {
                m_NewProjectError = "Failed to create project. Check if the path is accessible.";
            }
        }
    }
    ImGui::PopStyleColor(3);

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 14.0f);
    ImGui::EndPopup();
}

//////////////
// Viewport //
//////////////

bool Interface::ViewportResize()
{
    if (m_ViewportState.width != ImGui::GetWindowWidth() || m_ViewportState.height != ImGui::GetWindowHeight() ||
        m_ViewportState.x != ImGui::GetWindowPos().x || m_ViewportState.y != ImGui::GetWindowPos().y) {
        m_ViewportState.width = (uint)ImGui::GetWindowWidth();
        m_ViewportState.height = (uint)ImGui::GetWindowHeight();
        m_ViewportState.x = (uint)ImGui::GetWindowPos().x;
        m_ViewportState.y = (uint)ImGui::GetWindowPos().y;

        Application::s_Application->m_RenderAPI->SetViewport(m_ViewportState.width, m_ViewportState.height,
                                                             m_ViewportState.x, m_ViewportState.y);
        return true;
    }
    return false;
}
bool Interface::BeginViewport()
{
    if (m_ViewportState.visible) {
        ImGui::Begin(m_ViewportState.name.c_str(), nullptr,
                     ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);

        ImGui::Text("%d fps, %gms", Application::s_Application->m_FramesPerSecond,
                    Application::s_Application->m_FrameTimeMs);
        m_EditorProperties.m_ViewportHover = ImGui::IsWindowHovered();

        return true;
    }
    return false;
}

void Interface::EndViewport(RenderAPI& renderAPI, Ref<FrameBuffer> framebuffer)
{
    if (m_ViewportState.visible) {
        void* texID = renderAPI.GetFrameBufferImGuiTextureID(framebuffer);
#ifdef DD_API_VULKAN
        ImGui::Image(texID, ImVec2((float)m_ViewportState.width, (float)m_ViewportState.height),
                     ImVec2(0, 1), ImVec2(1, 0));
#else
        renderAPI.BindFrameBufferTexture(0, framebuffer);
        ImGui::Image(texID, ImVec2((float)m_ViewportState.width, (float)m_ViewportState.height),
                     ImVec2(0, 1), ImVec2(1, 0));
#endif
        ImGui::End();
    }
}