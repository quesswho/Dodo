#pragma once
#include "EditorIcons.h"
#include "PanelStates/EditorState.h"
#include "PanelStates/HierarchyState.h"
#include "PanelStates/InspectorState.h"

class HierarchyPanel {
  public:
    void Draw(EditorState& editorState, InspectorState& inspectorState, HierarchyState& state,
              const EditorIconSet& icons);

  private:
    void DrawIcon(void* texID, float sz);
};