#include "TransformGizmo.h"

#include "EditorWidgets.h"

#include <algorithm>
#include <cmath>

#include <imgui.h>

#include <ImGuizmo.h>

using namespace Dodo;
using namespace Math;

void TransformGizmo::BeginFrame()
{
    ImGuizmo::BeginFrame();
    m_Active = false;
}

void TransformGizmo::DrawToolbar(const EditorIcons& icons)
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 0.0f));

    if (EditorWidgets::ToolButton("Move", icons.Get(EditorIcon::Move), "Move (W)",
                                  m_Operation == GizmoOperation::Translate))
        m_Operation = GizmoOperation::Translate;
    ImGui::SameLine();
    if (EditorWidgets::ToolButton("Rotate", icons.Get(EditorIcon::Rotate), "Rotate (E)",
                                  m_Operation == GizmoOperation::Rotate))
        m_Operation = GizmoOperation::Rotate;
    ImGui::SameLine();
    if (EditorWidgets::ToolButton("Scale", icons.Get(EditorIcon::Scale), "Scale (R)",
                                  m_Operation == GizmoOperation::Scale))
        m_Operation = GizmoOperation::Scale;

    ImGui::SameLine();
    EditorWidgets::ToolSeparator();
    ImGui::SameLine();

    // Scaling always happens along the local axes, so the space only matters for moving and rotating.
    ImGui::BeginDisabled(m_Operation == GizmoOperation::Scale);
    if (EditorWidgets::ToolButton("Local", icons.Get(EditorIcon::Local), "Local space", m_Space == GizmoSpace::Local))
        m_Space = GizmoSpace::Local;
    ImGui::SameLine();
    if (EditorWidgets::ToolButton("World", icons.Get(EditorIcon::World), "World space", m_Space == GizmoSpace::World))
        m_Space = GizmoSpace::World;
    ImGui::EndDisabled();

    ImGui::PopStyleVar();
}

void TransformGizmo::HandleShortcuts()
{
    // Never switch operation in the middle of a drag, the matrix is applied according to the operation.
    if (ImGuizmo::IsUsing() || ImGui::GetIO().WantTextInput) return;

    if (ImGui::IsKeyPressed(ImGuiKey_W, false)) m_Operation = GizmoOperation::Translate;
    if (ImGui::IsKeyPressed(ImGuiKey_E, false)) m_Operation = GizmoOperation::Rotate;
    if (ImGui::IsKeyPressed(ImGuiKey_R, false)) m_Operation = GizmoOperation::Scale;
}

bool TransformGizmo::Manipulate(EditorState& editorState, const FreeCamera& camera, const Vec2& rectPos,
                                const Vec2& rectSize, bool interactive)
{
    m_Active = false;

    if (editorState.selection.Empty()) return false;

    auto& world = editorState.scene->GetWorld();
    const EntityID entity = editorState.selection.entities.front();
    if (!world.HasComponent<ModelComponent>(entity)) return false;

    Transformation& transformation = world.GetComponent<ModelComponent>(entity).m_Transformation;

    // A collapsed axis has no orientation left, so there is nothing sensible to draw or to drag.
    const float minScale = 1e-6f;
    if (std::abs(transformation.m_Scale.x) < minScale || std::abs(transformation.m_Scale.y) < minScale ||
        std::abs(transformation.m_Scale.z) < minScale)
        return false;

    ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
    switch (m_Operation) {
    case GizmoOperation::Translate:
        operation = ImGuizmo::TRANSLATE;
        break;
    case GizmoOperation::Rotate:
        operation = ImGuizmo::ROTATE;
        break;
    case GizmoOperation::Scale:
        operation = ImGuizmo::SCALE;
        break;
    }
    const ImGuizmo::MODE mode = m_Space == GizmoSpace::Local ? ImGuizmo::LOCAL : ImGuizmo::WORLD;

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(rectPos.x, rectPos.y, rectSize.x, rectSize.y);
    ImGuizmo::Enable(interactive);

    const Mat4 view = camera.GetViewMatrix();
    const Mat4 projection = camera.GetProjectionMatrix();
    Mat4 model = transformation.m_Model;

    const bool changed =
        ImGuizmo::Manipulate(view.m_Elements, projection.m_Elements, operation, mode, model.m_Elements);
    m_Active = ImGuizmo::IsOver() || ImGuizmo::IsUsing();

    if (changed) Apply(model, transformation);
    return changed;
}

void TransformGizmo::Apply(const Mat4& model, Transformation& transformation) const
{
    Vec3 axisX(model.m_Columns[0].x, model.m_Columns[0].y, model.m_Columns[0].z);
    Vec3 axisY(model.m_Columns[1].x, model.m_Columns[1].y, model.m_Columns[1].z);
    Vec3 axisZ(model.m_Columns[2].x, model.m_Columns[2].y, model.m_Columns[2].z);

    switch (m_Operation) {
    case GizmoOperation::Translate:
        transformation.m_Position = Vec3(model.m_Columns[3].x, model.m_Columns[3].y, model.m_Columns[3].z);
        break;
    case GizmoOperation::Scale:
        // The axis lengths are unsigned, carry over the sign so mirrored axes stay mirrored.
        transformation.m_Scale = Vec3(std::copysign(axisX.Magnitude(), transformation.m_Scale.x),
                                      std::copysign(axisY.Magnitude(), transformation.m_Scale.y),
                                      std::copysign(axisZ.Magnitude(), transformation.m_Scale.z));
        break;
    case GizmoOperation::Rotate: {
        // Dividing by the signed scale, rather than normalizing, leaves a pure rotation for mirrored axes too.
        axisX /= transformation.m_Scale.x;
        axisY /= transformation.m_Scale.y;
        axisZ /= transformation.m_Scale.z;

        // Transformation composes its rotation as Rz * Rx * Ry, extract the angles in that order.
        Vec3 rotation;
        rotation.x = std::asin(std::clamp(axisY.z, -1.0f, 1.0f));
        if (std::abs(axisY.z) < 0.999999f) {
            rotation.y = std::atan2(-axisX.z, axisZ.z);
            rotation.z = std::atan2(-axisY.x, axisY.y);
        } else {
            // Gimbal lock: Y and Z turn around the same axis, so put the whole turn on Y.
            rotation.y = std::atan2(axisZ.x, axisX.x);
            rotation.z = 0.0f;
        }
        transformation.m_Rotation = rotation;
        break;
    }
    }

    transformation.Calculate();
}
