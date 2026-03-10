#pragma once
#include "PanelStates/EditorState.h"
#include "PanelStates/InspectorState.h"
#include "PanelStates/HierarchyState.h"

class HierarchyPanel {
  public:
    void Draw(EditorState& editorState, InspectorState& inspectorState, HierarchyState& state);
};