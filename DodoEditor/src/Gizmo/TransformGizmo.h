#pragma once

#include "EditorIcons.h"
#include "PanelStates/EditorState.h"

#include <Dodo.h>

enum class GizmoOperation {
    Translate,
    Rotate,
    Scale
};

enum class GizmoSpace {
    Local,
    World
};

/**
 * Translate, rotate and scale gizmo drawn on top of the scene viewport.
 *
 * Manipulates the ModelComponent transformation of the first selected entity, which is the same entity
 * the inspector edits.
 */
class TransformGizmo {
  public:
    /**
     * Resets the per frame gizmo state. Must be called once per frame, right after the ImGui frame has started.
     */
    void BeginFrame();

    /**
     * Draws the operation and space selectors as a row of icon buttons at the cursor of the current window.
     */
    void DrawToolbar(const EditorIcons& icons);

    /**
     * Switches operation from the keyboard: W translates, E rotates and R scales.
     * Call only while the viewport owns the keyboard, so the keys do not clash with other input.
     */
    void HandleShortcuts();

    /**
     * Draws the gizmo into the current window and applies any manipulation to the selected entity.
     *
     * @param editorState Scene and selection to operate on.
     * @param camera Camera the scene in the viewport was rendered with.
     * @param rectPos Top left corner of the scene image, in screen coordinates.
     * @param rectSize Size of the scene image, in pixels.
     * @param interactive False to draw the gizmo without letting the mouse grab it.
     * @return True if the transformation was changed this frame.
     */
    bool Manipulate(EditorState& editorState, const Math::FreeCamera& camera, const Math::Vec2& rectPos,
                    const Math::Vec2& rectSize, bool interactive);

    /**
     * @return True if the mouse was over the gizmo or dragging it during the last Manipulate call.
     */
    bool IsActive() const { return m_Active; }

  private:
    /**
     * Writes the manipulated model matrix back into the transformation. Only the component that belongs to
     * the current operation is updated, so a drag never disturbs the other two.
     */
    void Apply(const Math::Mat4& model, Math::Transformation& transformation) const;

    GizmoOperation m_Operation = GizmoOperation::Translate;
    GizmoSpace m_Space = GizmoSpace::Local;
    bool m_Active = false;
};
