#pragma once

#include <Dodo.h>

#include "Data/EditorSceneFile.h"
#include "PanelStates/EditorState.h"
#include "PanelStates/HierarchyState.h"
#include "Panels/AssetBrowserPanel.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Scene/EditorScene.h"

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

    InspectorPanel m_InspectorPanel;
    HierarchyPanel m_HierarchyPanel;
    AssetBrowserPanel m_AssetBrowserPanel;

  public:
    Interface(EditorScene* scene);

    bool BeginDraw();
    bool BeginViewport();
    bool ViewportResize();
    void EndViewport(FrameBuffer* framebuffer);
    void EndDraw();

    void ChangeScene(EditorScene* scene);

  private:
    void InitInterface();

    void ResetDockspace(uint dockspace_id);

    bool m_ChangeScene;

    EditorSceneFile fileReader;
};