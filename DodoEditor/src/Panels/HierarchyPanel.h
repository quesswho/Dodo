#pragma once
#include "PanelStates/EditorState.h"
#include "PanelStates/HierarchyState.h"
#include "PanelStates/InspectorState.h"

class HierarchyPanel {
  public:
    void Draw(EditorState& editorState, InspectorState& inspectorState, HierarchyState& state);
};