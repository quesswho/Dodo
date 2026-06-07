#pragma once

#include <Dodo.h>

#include "Data/EditorSceneFile.h"
#include "EditorIcons.h"
#include "PanelStates/EditorState.h"
#include "PanelStates/HierarchyState.h"
#include "Panels/AssetBrowserPanel.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Project/Project.h"
#include "Scene/EditorScene.h"

#include <filesystem>
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

  public:
    Interface(EditorScene* scene);

    bool BeginDraw();
    bool BeginViewport();
    bool ViewportResize();
    void EndViewport(RenderAPI& renderAPI, Ref<FrameBuffer> framebuffer);
    void EndDraw();

    void ChangeScene(EditorScene* scene);

  private:
    void InitInterface();
    void ResetDockspace(uint dockspace_id);
    void SetActiveProject(std::unique_ptr<Dodo::Project> project);
    void DrawNewProjectModal();

    bool m_ChangeScene = false;

    std::unique_ptr<Dodo::Project> m_Project;
    EditorSceneFile fileReader;

    char m_NewProjectNameBuf[256] = {};
    std::filesystem::path m_NewProjectDir;
    std::string m_NewProjectError;
};