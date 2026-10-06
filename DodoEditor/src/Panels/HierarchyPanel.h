#pragma once
#include "EditorIcons.h"
#include "PanelStates/EditorState.h"
#include "PanelStates/HierarchyState.h"
#include "PanelStates/InspectorState.h"

class HierarchyPanel {
  public:
    void Draw(EditorState& editorState, InspectorState& inspectorState, HierarchyState& state,
              const EditorIcons& icons);

  private:
    /**
     * Draws the selectable row of one entity: icon, name and its context menu.
     *
     * @return True when deleting the selection was requested from the context menu.
     */
    bool DrawEntityRow(EditorState& editorState, InspectorState& inspectorState, EntityID entityId,
                       const std::string& name, const EditorIcons& icons);

    /**
     * Draws the text field that replaces the row of the entity being renamed.
     */
    void DrawRenameRow(EditorState& editorState, EntityID entityId);

    void CreateEntity(EditorState& editorState);
    void DeleteSelection(EditorState& editorState, InspectorState& inspectorState);

    static std::string GetEntityName(EditorWorld& world, EntityID entityId);
};
