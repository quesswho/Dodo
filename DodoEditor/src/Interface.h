#pragma once

#include <Dodo.h>

#include "PanelStates/EditorState.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "PanelStates/HierarchyState.h"


struct Component {
    Component() : m_Name("None"), m_Selected(false) {}

    Component(const char* name) : m_Name(name), m_Selected(false) {}

    const char* m_Name;
    bool m_Selected;
};

struct EditorProperties {
    bool m_ShowViewport;

    bool m_ViewportInput;
    bool m_ViewportHover;

    const char* m_HierarchyName;
    const char* m_ViewportName;
    const char* m_InspectorName;
};

class Interface {
  public:
    EditorProperties m_EditorProperties;

    EditorState m_EditorState;
    ViewportState m_ViewportState;
    InspectorState m_InspectorState;
    HierarchyState m_HierarchyState;

    InspectorPanel m_InspectorPanel;
    HierarchyPanel m_HierarchyPanel;

  public:
    Interface(Scene* scene);

    bool BeginDraw();
    bool BeginViewport();
    bool ViewportResize();
    void EndViewport(FrameBuffer* framebuffer);
    void EndDraw();

    void ChangeScene(Scene* scene);

  private:
    void InitInterface();

    void ResetDockspace(uint dockspace_id);

    std::vector<Component> m_HierarchyComponents;
    std::vector<Component> m_InspectorComponents;

    bool m_ChangeScene;

    SceneFile m_File;
};