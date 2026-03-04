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
    ImVec4 aa = style.Colors[ImGuiCol_TabActive];
    ImVec4 aa2 = style.Colors[ImGuiCol_TitleBgActive];
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

    m_EditorProperties.m_ViewportName = "Viewport";
    m_EditorProperties.m_HierarchyName = "Hierarchy";
    m_EditorProperties.m_InspectorName = "Inspector";

    m_EditorProperties.m_ViewportHover = false;
    m_EditorProperties.m_ViewportInput = false;

    m_EditorProperties.m_ShowViewport = true;

    // Hierarchy
    m_HierarchyComponents.push_back(Component("ModelComponent"));
    m_HierarchyState.visible = true;

    // Inspector
    m_InspectorComponents = m_HierarchyComponents;

    m_InspectorState.visible = true;
    m_InspectorState.dirty = false;
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

    ImGui::Begin("DockSpace", 0, dockWindow_flags);
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
                if (ImGui::MenuItem("Scene")) {
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Open")) {
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
            ImGui::MenuItem(m_EditorProperties.m_ViewportName, "", &m_EditorProperties.m_ShowViewport);
            ImGui::MenuItem(m_EditorProperties.m_HierarchyName, "", &m_HierarchyState.visible);
            ImGui::MenuItem(m_EditorProperties.m_InspectorName, "", &m_InspectorState.visible);
            ImGui::Separator();
            if (ImGui::Button("Reset DockSpace")) {
                s_ResetDockspace = true;
            }
            ImGui::EndMenu();
        }
        ImGui::PopStyleColor();
        ImGui::EndMenuBar();
    }
    ImGui::End();

    if (m_HierarchyState.visible) m_HierarchyPanel.Draw(m_EditorState, m_InspectorState);
    if (m_InspectorState.visible) m_InspectorPanel.Draw(m_EditorState, m_InspectorState);

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
    if (m_ViewportState.width != ImGui::GetWindowWidth() || m_ViewportState.height != ImGui::GetWindowHeight() ||
        m_ViewportState.x != ImGui::GetWindowPos().x || m_ViewportState.y != ImGui::GetWindowPos().y) {
        m_ViewportState.width = (uint)ImGui::GetWindowWidth();
        m_ViewportState.height = (uint)ImGui::GetWindowHeight();
        m_ViewportState.x = (uint)ImGui::GetWindowPos().x;
        m_ViewportState.y = (uint)ImGui::GetWindowPos().y;

        Application::s_Application->m_RenderAPI->ResizeDefaultViewport(m_ViewportState.width, m_ViewportState.height,
                                                                       m_ViewportState.x, m_ViewportState.y);
        return true;
    }
    return false;
}
bool Interface::BeginViewport()
{
    if (m_EditorProperties.m_ShowViewport) {
        static ImGuiWindowFlags s_ViewportWindow = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
        ImGui::Begin("Viewport", 0, s_ViewportWindow);

        ImGui::Text("%d fps, %gms", Application::s_Application->m_FramesPerSecond,
                    Application::s_Application->m_FrameTimeMs);
        m_EditorProperties.m_ViewportHover = ImGui::IsWindowHovered();

        return true;
    }
    return false;
}

void Interface::EndViewport(FrameBuffer* framebuffer)
{
    if (m_EditorProperties.m_ShowViewport) {
        ImGui::Image((void*)(intptr_t)framebuffer->GetTextureHandle(),
                     ImVec2((float)m_ViewportState.width, (float)m_ViewportState.height), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();
    }
}