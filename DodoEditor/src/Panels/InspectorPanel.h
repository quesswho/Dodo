#pragma once
#include "PanelStates/EditorState.h"
#include "PanelStates/InspectorState.h"

class InspectorPanel {
  public:
    void Draw(EditorState& state, InspectorState& inspector);
};