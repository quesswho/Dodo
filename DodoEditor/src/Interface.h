#pragma once

#include <Dodo.h>

#include "Data/EditorSceneFile.h"
#include "EditorIcons.h"
#include "Gizmo/TransformGizmo.h"
#include "PanelStates/EditorState.h"
#include "PanelStates/HierarchyState.h"
#include "Panels/AssetBrowserPanel.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Project/Project.h"
#include "Scene/EditorScene.h"

#include <filesystem>
#include <imgui.h>
#include <string>

struct EditorProperties {
    bool m_ViewportInput;
    bool m_ViewportHover;
};

class Interface {
  public:
    EditorProperties m_EditorProperties;

    EditorState m_EditorState;
    ViewportState m_ViewportState;
    InspectorState m_InspectorState;
    HierarchyState m_HierarchyState;
    AssetBrowserState m_AssetBrowserState;

    EditorIcons m_Icons;

    InspectorPanel m_InspectorPanel;
    HierarchyPanel m_HierarchyPanel;
    AssetBrowserPanel m_AssetBrowserPanel;

    TransformGizmo m_TransformGizmo;

  public:
    Interface(EditorScene* scene);

    bool BeginDraw();
    bool BeginViewport();
    bool ViewportResize();
    void EndViewport(RenderAPI& renderAPI, Ref<FrameBuffer> framebuffer, const Math::FreeCamera& camera);
    void EndDraw();

    void ChangeScene(EditorScene* scene);

  private:
    void InitInterface();
    void ResetDockspace(uint dockspace_id);
    void SetActiveProject(std::unique_ptr<Dodo::Project> project);
    void DrawNewProjectModal();

    /**
     * Draws the strip along the bottom of the window with the project, the scene size and a camera hint.
     */
    void DrawStatusBar();

    /**
     * Draws the floating gizmo toolbar and the frame statistics on top of the scene image.
     *
     * @param imagePos Top left corner of the scene image, in screen coordinates.
     * @param imageSize Size of the scene image, in pixels.
     */
    void DrawViewportOverlay(const ImVec2& imagePos, const ImVec2& imageSize);

    bool m_ChangeScene = false;
    bool m_ViewportToolbarHovered = false; // From the previous frame, the gizmo is handled before the toolbar

    std::unique_ptr<Dodo::Project> m_Project;
    EditorSceneFile fileReader;

    char m_NewProjectNameBuf[256] = {};
    std::filesystem::path m_NewProjectDir;
    std::string m_NewProjectError;
};