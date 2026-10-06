#include "Interface.h"

#include "Data/EditorSceneFile.h"
#include "EditorTheme.h"
#include "EditorWidgets.h"
#include "FileDialog.h"

#include <algorithm>

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
    EditorTheme::Apply();

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
    m_InspectorState.visible = true;
    m_InspectorState.dirty = false;

    // Asset Browser
    m_AssetBrowserState.name = "Asset Browser";
    m_AssetBrowserState.visible = true;

    m_Icons.Load(*Application::s_Application->m_RenderAPI);
}

bool Interface::BeginDraw()
{
    Application::s_Application->ImGuiNewFrame();
    m_TransformGizmo.BeginFrame();

    static bool s_ResetDockspace = false;

    // The status bar takes its strip from the work area, so it has to come before the dockspace window.
    DrawStatusBar();

    ImGuiWindowFlags dockWindow_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    dockWindow_flags |=
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    dockWindow_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGuiIO& io = ImGui::GetIO();

    // No padding: the panels reach the window edges and are only separated by the docking splitters.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, EditorTheme::Background);
    ImGui::Begin("DockSpace", nullptr, dockWindow_flags);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("DockSpace");
        if (ImGui::DockBuilderGetNode(dockspace_id) == NULL || s_ResetDockspace) {
            ResetDockspace(dockspace_id);
            s_ResetDockspace = false;
        }
        // Panels are closed from their tab, a second close button per dock node is only clutter.
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_NoCloseButton);
    }

    if (ImGui::BeginMenuBar()) {
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
            if (ImGui::MenuItem("Reset Layout")) {
                s_ResetDockspace = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    DrawNewProjectModal();

    ImGui::End();

    m_HierarchyPanel.Draw(m_EditorState, m_InspectorState, m_HierarchyState, m_Icons);
    m_InspectorPanel.Draw(m_EditorState, m_InspectorState, m_Icons);
    m_AssetBrowserPanel.Draw(m_AssetBrowserState, m_Icons);

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

    ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.17f, nullptr, &dock_main_id);
    ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);
    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.28f, nullptr, &dock_main_id);

    ImGui::DockBuilderDockWindow(m_ViewportState.name.c_str(), dock_main_id);
    ImGui::DockBuilderDockWindow(m_HierarchyState.name.c_str(), dock_left);
    ImGui::DockBuilderDockWindow(m_InspectorState.name.c_str(), dock_right);
    ImGui::DockBuilderDockWindow(m_AssetBrowserState.name.c_str(), dock_bottom);

    ImGui::DockBuilderFinish(dockspace_id);
}

void Interface::DrawStatusBar()
{
    const ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    if (ImGui::BeginViewportSideBar("##StatusBar", ImGui::GetMainViewport(), ImGuiDir_Down, ImGui::GetFrameHeight(),
                                    flags)) {
        if (ImGui::BeginMenuBar()) {
            ImGui::PushFont(nullptr, EditorTheme::FontSizeSmall);
            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::TextDim);

            if (m_Project)
                ImGui::Text("Project: %s", m_Project->m_Name.c_str());
            else
                ImGui::TextUnformatted("No project open");

            const size_t entities = m_EditorState.scene->GetWorld().GetAliveEntities().size();
            ImGui::SameLine(0.0f, 20.0f);
            ImGui::Text("%zu %s", entities, entities == 1 ? "entity" : "entities");

            const char* hint = m_EditorProperties.m_ViewportInput ? "Flying the camera, press Z to release the cursor"
                                                                  : "Hover the viewport and press Z to fly the camera";
            const float hintWidth = ImGui::CalcTextSize(hint).x + ImGui::GetStyle().WindowPadding.x;
            ImGui::SameLine(0.0f, 20.0f);
            ImGui::SetCursorPosX(std::max(ImGui::GetWindowWidth() - hintWidth, ImGui::GetCursorPosX()));
            ImGui::TextUnformatted(hint);

            ImGui::PopStyleColor();
            ImGui::PopFont();
            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
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

    // Accent header band
    ImGui::PushStyleColor(ImGuiCol_ChildBg, EditorTheme::Accent);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    if (ImGui::BeginChild("##NPHdr", ImVec2(0.0f, 52.0f), false, ImGuiWindowFlags_NoScrollbar)) {
        ImGui::SetCursorPos(ImVec2(18.0f, 16.0f));
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Create New Project");
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
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
    ImGui::SetNextItemWidth(contentW - browseW - sty.ItemSpacing.x);
    ImGui::InputText("##NPLoc", locBuf, sizeof(locBuf), ImGuiInputTextFlags_ReadOnly);
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
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::TextDim);
        auto preview = m_NewProjectDir / m_NewProjectNameBuf;
        ImGui::TextWrapped("Will be created at: %s", preview.generic_string().c_str());
        ImGui::PopStyleColor();
    }

    // Error message
    if (!m_NewProjectError.empty()) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);
        ImGui::SetCursorPosX(pad);
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::TextError);
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
        ImGui::GetColorU32(EditorTheme::Border));
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
    ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Accent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::AccentHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  EditorTheme::AccentActive);
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
    if (!m_ViewportState.visible) return false;
    
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 size = ImGui::GetContentRegionAvail();

    // A collapsed or very small window leaves no room, keep the framebuffer at a valid size.
    const uint width = (uint)std::max(size.x, 1.0f);
    const uint height = (uint)std::max(size.y, 1.0f);
    const uint x = (uint)std::max(pos.x, 0.0f);
    const uint y = (uint)std::max(pos.y, 0.0f);

    if (m_ViewportState.width != width || m_ViewportState.height != height || m_ViewportState.x != x ||
        m_ViewportState.y != y) {
        m_ViewportState.width = width;
        m_ViewportState.height = height;
        m_ViewportState.x = x;
        m_ViewportState.y = y;

        Application::s_Application->m_RenderAPI->SetViewport(m_ViewportState.width, m_ViewportState.height,
                                                             m_ViewportState.x, m_ViewportState.y);
        return true;
    }
    return false;
}
bool Interface::BeginViewport()
{
    if (m_ViewportState.visible) {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
        // Dragging a gizmo handle must not drag a floating viewport window along with it.
        if (m_TransformGizmo.IsActive()) flags |= ImGuiWindowFlags_NoMove;

        // The scene image fills the whole panel, the toolbar floats on top of it.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin(m_ViewportState.name.c_str(), nullptr, flags);
        ImGui::PopStyleVar();

        m_EditorProperties.m_ViewportHover = ImGui::IsWindowHovered();
        if (m_EditorProperties.m_ViewportHover && !m_EditorProperties.m_ViewportInput)
            m_TransformGizmo.HandleShortcuts();

        return true;
    }
    return false;
}

void Interface::EndViewport(RenderAPI& renderAPI, Ref<FrameBuffer> framebuffer, const FreeCamera& camera)
{
    if (m_ViewportState.visible) {
        void* texID = renderAPI.GetFrameBufferImGuiTextureID(framebuffer);
        ImGui::Image(texID, ImVec2((float)m_ViewportState.width, (float)m_ViewportState.height));

        const ImVec2 imagePos = ImGui::GetItemRectMin();
        const ImVec2 imageSize = ImGui::GetItemRectSize();
        // The gizmo stays visible while flying the camera, but the hidden cursor must not grab it.
        // A click on the toolbar must not reach a gizmo handle behind it either.
        const bool interactive = !m_EditorProperties.m_ViewportInput && !m_ViewportToolbarHovered;
        if (m_TransformGizmo.Manipulate(m_EditorState, camera, Vec2(imagePos.x, imagePos.y),
                                        Vec2(imageSize.x, imageSize.y), interactive)) {
            m_InspectorState.dirty = true; // Refresh the inspector fields from the new transformation
        }

        DrawViewportOverlay(imagePos, imageSize);
        ImGui::End();
    }
}

void Interface::DrawViewportOverlay(const ImVec2& imagePos, const ImVec2& imageSize)
{
    const float margin = 10.0f;
    const float padding = 4.0f;
    const float rounding = 6.0f;
    const ImU32 background =
        ImGui::GetColorU32(ImVec4(EditorTheme::Panel.x, EditorTheme::Panel.y, EditorTheme::Panel.z, 0.90f));
    const ImU32 border = ImGui::GetColorU32(EditorTheme::Border);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // The size of the toolbar is only known once its buttons are laid out, but its background has to be drawn
    // below them. Splitting the draw list lets the background be added afterwards.
    drawList->ChannelsSplit(2);
    drawList->ChannelsSetCurrent(1);
    ImGui::SetCursorScreenPos(ImVec2(imagePos.x + margin + padding, imagePos.y + margin + padding));
    ImGui::BeginGroup();
    m_TransformGizmo.DrawToolbar(m_Icons);
    ImGui::EndGroup();

    const ImVec2 toolbarMin = ImVec2(ImGui::GetItemRectMin().x - padding, ImGui::GetItemRectMin().y - padding);
    const ImVec2 toolbarMax = ImVec2(ImGui::GetItemRectMax().x + padding, ImGui::GetItemRectMax().y + padding);
    drawList->ChannelsSetCurrent(0);
    drawList->AddRectFilled(toolbarMin, toolbarMax, background, rounding);
    drawList->AddRect(toolbarMin, toolbarMax, border, rounding);
    drawList->ChannelsMerge();

    m_ViewportToolbarHovered = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(toolbarMin, toolbarMax);

    // Frame statistics in the opposite corner, left out when they would run into the toolbar.
    char stats[64];
    snprintf(stats, sizeof(stats), "%u fps   %.1f ms", Application::s_Application->m_FramesPerSecond,
             Application::s_Application->m_FrameTimeMs);

    ImGui::PushFont(nullptr, EditorTheme::FontSizeSmall);
    const ImVec2 textSize = ImGui::CalcTextSize(stats);
    const ImVec2 statsMax =
        ImVec2(imagePos.x + imageSize.x - margin, imagePos.y + margin + textSize.y + padding * 2.0f);
    const ImVec2 statsMin = ImVec2(statsMax.x - textSize.x - padding * 4.0f, imagePos.y + margin);
    if (statsMin.x > toolbarMax.x + margin) {
        drawList->AddRectFilled(statsMin, statsMax, background, rounding);
        drawList->AddRect(statsMin, statsMax, border, rounding);
        drawList->AddText(ImVec2(statsMin.x + padding * 2.0f, statsMin.y + padding),
                          ImGui::GetColorU32(EditorTheme::TextDim), stats);
    }
    ImGui::PopFont();
}
