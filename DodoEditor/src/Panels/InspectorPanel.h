#pragma once
#include "EditorIcons.h"
#include "PanelStates/EditorState.h"
#include "PanelStates/InspectorState.h"

class InspectorPanel {
  public:
    void Draw(EditorState& state, InspectorState& inspector, const EditorIcons& icons);

  private:
    /**
     * Draws the icon, the editable name and the id of the inspected entity.
     */
    void DrawHeader(EditorState& state, InspectorState& inspector, EntityID entityId, const EditorIcons& icons);

    void DrawModelComponent(EditorWorld& world, InspectorState& inspector, EntityID entityId);

    /**
     * Draws the entries of the menu that adds components to the entity. Must be called inside of a popup.
     */
    void DrawAddComponentMenu(EditorWorld& world, EntityID entityId);
};
