#include "InspectorPanel.h"

#include "EditorTheme.h"
#include "EditorWidgets.h"
#include "FileDialog.h"

#include <Dodo.h>
#include <algorithm>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void InspectorPanel::Draw(EditorState& editorState, InspectorState& state, const EditorIcons& icons)
{
    if (!state.visible) return;

    if (!ImGui::Begin(state.name.c_str(), &state.visible)) {
        ImGui::End();
        return;
    }

    if (editorState.selection.Empty()) {
        state.nameActive = false;
        EditorWidgets::EmptyState("Select an entity to inspect it.");
        ImGui::End();
        return;
    }

    auto& world = editorState.scene->GetWorld();
    const EntityID entityId = editorState.selection.entities.front();

    DrawHeader(editorState, state, entityId, icons);

    ImGui::Dummy(ImVec2(0.0f, 2.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 2.0f));

    if (ImGui::BeginPopupContextWindow("##inspectorContext",
                                       ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        if (ImGui::BeginMenu("Add Component")) {
            DrawAddComponentMenu(world, entityId);
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }

    if (world.HasComponent<ModelComponent>(entityId)) DrawModelComponent(world, state, entityId);

    if (!world.HasAnyComponent(entityId)) {
        ImGui::TextColored(EditorTheme::TextDim, "This entity has no components.");
    }

    ImGui::Dummy(ImVec2(0.0f, 6.0f));
    const float buttonWidth = std::min(180.0f, ImGui::GetContentRegionAvail().x);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - buttonWidth) * 0.5f);
    if (ImGui::Button("Add Component", ImVec2(buttonWidth, 0.0f))) ImGui::OpenPopup("##addComponent");
    if (ImGui::BeginPopup("##addComponent")) {
        DrawAddComponentMenu(world, entityId);
        ImGui::EndPopup();
    }

    ImGui::End();
}

void InspectorPanel::DrawHeader(EditorState& editorState, InspectorState& state, EntityID entityId,
                                const EditorIcons& icons)
{
    auto& world = editorState.scene->GetWorld();
    const ImGuiStyle& style = ImGui::GetStyle();

    // Follow the entity, except while the user is typing a new name.
    if (!state.nameActive) {
        state.nameEntity = entityId;
        state.nameBuffer = world.HasComponent<NameComponent>(entityId)
                               ? world.GetComponent<NameComponent>(entityId).name
                               : "Entity_" + std::to_string(entityId);
    }

    if (void* icon = icons.Get(EditorIcon::Entity)) {
        const float iconSize = 18.0f;
        const float frameHeight = ImGui::GetFrameHeight();
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const ImVec2 iconMin = ImVec2(pos.x, pos.y + std::floor((frameHeight - iconSize) * 0.5f));
        ImGui::GetWindowDrawList()->AddImage(icon, iconMin, ImVec2(iconMin.x + iconSize, iconMin.y + iconSize),
                                             ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
                                             ImGui::GetColorU32(EditorTheme::AccentHover));
        ImGui::Dummy(ImVec2(iconSize, frameHeight));
        ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##name", &state.nameBuffer, ImGuiInputTextFlags_CharsNoBlank);
    state.nameActive = ImGui::IsItemActive();

    const bool nameEntityAlive = world.GetAliveEntities().count(state.nameEntity) > 0;
    if (ImGui::IsItemDeactivatedAfterEdit() && !state.nameBuffer.empty() && nameEntityAlive) {
        if (!world.HasComponent<NameComponent>(state.nameEntity))
            world.AddComponent<NameComponent>(state.nameEntity, NameComponent{state.nameBuffer});
        else
            world.GetComponent<NameComponent>(state.nameEntity).name = state.nameBuffer;
    }

    ImGui::PushFont(nullptr, EditorTheme::FontSizeSmall);
    const size_t selected = editorState.selection.entities.size();
    if (selected > 1)
        ImGui::TextColored(EditorTheme::TextDim, "Entity %u, and %zu more selected", entityId, selected - 1);
    else
        ImGui::TextColored(EditorTheme::TextDim, "Entity %u", entityId);
    ImGui::PopFont();
}

void InspectorPanel::DrawModelComponent(EditorWorld& world, InspectorState& state, EntityID entityId)
{
    if (!EditorWidgets::SectionHeader("Model")) return;

    ModelComponent& model = world.GetComponent<ModelComponent>(entityId);
    AssetManager& assets = *Application::s_Application->m_AssetManager;

    TransformEditState& tState = state.transformState;
    if (state.dirty) {
        tState.translate = model.m_Transformation.m_Position;
        tState.scale = model.m_Transformation.m_Scale;
        tState.rotate = Math::Vec3(Math::ToDegrees(model.m_Transformation.m_Rotation.x),
                                   Math::ToDegrees(model.m_Transformation.m_Rotation.y),
                                   Math::ToDegrees(model.m_Transformation.m_Rotation.z));
        state.dirty = false;
    }

    if (!EditorWidgets::BeginProperties("##model")) return;

    EditorWidgets::Property("Mesh");
    {
        const ImGuiStyle& style = ImGui::GetStyle();
        const float browseWidth = ImGui::GetFrameHeight() + style.FramePadding.x;
        ImGui::SetNextItemWidth(
            std::max(ImGui::GetContentRegionAvail().x - browseWidth - style.ItemInnerSpacing.x, 1.0f));

        // Check if it's a built-in model first to avoid error logs
        std::string source = "Built-in model";
        bool missing = false;
        if (assets.HasPath(model.m_ModelID)) {
            source = assets.GetModelPath(model.m_ModelID);
            if (source.empty()) {
                source = "Path not found";
                missing = true;
            }
        }
        if (missing) ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::TextError);
        ImGui::InputText("##source", &source, ImGuiInputTextFlags_ReadOnly);
        if (missing) ImGui::PopStyleColor();

        ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
        if (ImGui::Button("...", ImVec2(browseWidth, 0.0f))) {
            std::filesystem::path path = FileDialog::OpenFile("Open file", "Model\0*.fbx;*.obj\0");
            if (!path.empty()) {
                // Replace the component with new model
                ModelID id = assets.LoadModel(path.string());
                world.GetComponent<ModelComponent>(entityId) = ModelComponent(id, model.m_Transformation);
            }
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) ImGui::SetTooltip("Browse for a model file");
    }

    EditorWidgets::Property("Position");
    if (EditorWidgets::Vec3Control("##translate", tState.translate, 0.05f, "%.3f")) {
        model.m_Transformation.Move(tState.translate);
    }

    EditorWidgets::Property("Rotation");
    if (EditorWidgets::Vec3Control("##rotate", tState.rotate, 0.5f, "%.3f")) {
        tState.rotate = Math::Vec3(std::fmod(tState.rotate.x, 360.0f), std::fmod(tState.rotate.y, 360.0f),
                                   std::fmod(tState.rotate.z, 360.0f));
        model.m_Transformation.Rotate(tState.rotate);
    }

    EditorWidgets::Property("Scale");
    if (EditorWidgets::Vec3Control("##scale", tState.scale, 0.0001f, "%.4f")) {
        if (tState.syncScale) {
            Math::Vec3 diff = tState.scale - model.m_Transformation.m_Scale;
            model.m_Transformation.m_Scale += diff.x + diff.y + diff.z;
            tState.scale = model.m_Transformation.m_Scale;
            model.m_Transformation.Calculate(); // Recompute model matrix after manual change
        } else {
            model.m_Transformation.Scale(tState.scale);
        }
    }

    EditorWidgets::Property("");
    ImGui::Checkbox("Uniform scale", &tState.syncScale);

    EditorWidgets::EndProperties();
}

void InspectorPanel::DrawAddComponentMenu(EditorWorld& world, EntityID entityId)
{
    AssetManager& assets = *Application::s_Application->m_AssetManager;

    if (ImGui::BeginMenu("Geometry")) {
        if (ImGui::MenuItem("Browse..")) {
            std::filesystem::path path = FileDialog::OpenFile("Open file", "Model\0*.fbx;*.obj\0");
            if (!path.empty()) {
                // Replace the component with new model
                ModelID id = assets.LoadModel(path.string());
                world.AddComponent<ModelComponent>(entityId, ModelComponent(id, Math::Transformation()));
            }
        }
        // TODO: Take list of strings and BuiltinModel enum values from AssetManager for better scalability
        if (ImGui::MenuItem("Cube")) {
            if (!world.HasComponent<ModelComponent>(entityId)) {
                ModelID id = assets.GetBuiltinModel(BuiltinModel::Cube);
                world.AddComponent<ModelComponent>(entityId, ModelComponent(id, Math::Transformation()));
            }
        }
        if (ImGui::MenuItem("Terrain")) {
            if (!world.HasComponent<ModelComponent>(entityId)) {
                ModelID id = assets.GetBuiltinModel(BuiltinModel::Terrain);
                world.AddComponent<ModelComponent>(entityId, ModelComponent(id, Math::Transformation()));
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Script")) {
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Audio")) {
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Other")) {
        ImGui::EndMenu();
    }
}
