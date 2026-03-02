#pragma once

#include <Dodo.h>

using namespace Dodo;

struct Component {
    Component() : m_Name("None"), m_Selected(false) {}

    Component(const char *name) : m_Name(name), m_Selected(false) {}

    const char *m_Name;
    bool m_Selected;
};

struct EditorProperties {
    bool m_ShowViewport;
    bool m_ShowHierarchy;
    bool m_ShowInspector;

    bool m_ViewportHover;
    bool m_ViewportInput;

    const char *m_HierarchyName;
    const char *m_ViewportName;
    const char *m_InspectorName;
};

struct Selection {
    std::vector<EntityID> entities;

    bool Empty() const { return entities.empty(); }
    bool Single() const { return entities.size() == 1; }
    bool Contains(EntityID e) const { return std::find(entities.begin(), entities.end(), e) != entities.end(); }
};

struct EditorState {
    Scene* scene = nullptr;
    Selection selection;
}

struct ViewportState {
    uint width = 0, height = 0;
    uint x = 0, y = 0;
};

struct InspectorState {
    bool dirty = false;
    std::string nameBuffer;
};

class Interface {
  public:
    EditorProperties m_EditorProperties;

    EditorState m_EditorState;
    ViewportState m_ViewportState;
    InspectorState m_InspectorState;

  public:
    Interface(Scene *scene);

    bool BeginDraw();
    bool BeginViewport();
    bool ViewportResize();
    void EndViewport(FrameBuffer *framebuffer);
    void EndDraw();

    void ChangeScene(Scene *scene);

    void SetChangeSceneCallback(void (*scene)(Scene *));

  private:
    void InitInterface();

    void ResetDockspace(uint dockspace_id);

    void DrawHierarchy();
    void DrawInspector();

    void SingleSelect(EntityID e);
    void ToggleSelect(EntityID e);
    void ClearSelection();

    std::vector<Component> m_HierarchyComponents;
    std::vector<Component> m_InspectorComponents;

    bool m_ChangeScene;

    SceneFile m_File;

    void (*m_ChangeSceneFunc)(Scene *);
};