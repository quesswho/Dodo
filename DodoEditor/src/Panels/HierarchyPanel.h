#pragma once
#include "PanelStates/EditorState.h"
#include "PanelStates/InspectorState.h"

class HierarchyPanel {
  public:
    void Draw(EditorState& state, InspectorState& inspector);
};