#include "HierarchyPanel.h"

#include <Dodo.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void HierarchyPanel::DrawIcon(void* texID, float sz)
{
    if (texID)
        ImGui::Image((ImTextureID)texID, ImVec2(sz, sz));
    else
        ImGui::Dummy(ImVec2(sz, sz));
    ImGui::SameLine();
}

void HierarchyPanel::Draw(EditorState& editorState, InspectorState& inspectorState, HierarchyState& state,
                          const EditorIconSet& icons)
{
    if (!ImGui::Begin(state.name.c_str(), &state.visible)) {
        ImGui::End();
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete) && !editorState.renameState.isActive() &&
        !editorState.selection.Empty()) {
        auto& world = editorState.scene->GetWorld();
        for (EntityID id : editorState.selection.entities)
            world.DeleteEntity(id);
        editorState.selection.Clear();
        inspectorState.dirty = false;
        inspectorState.visible = false;
    }

    if (ImGui::BeginPopupContextWindow("Right-Click Hierarchy", ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::MenuItem("Create New")) {
            EntityID newEntity = editorState.scene->GetWorld().CreateEntity();
            editorState.renameState.Begin(editorState.scene->GetWorld(), newEntity);
        }
        ImGui::EndPopup();
    }

    float iconSz = ImGui::GetTextLineHeight();
    DrawIcon(icons.entityRoot, iconSz);

    if (ImGui::TreeNodeEx("Entities", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& world = editorState.scene->GetWorld();
        if (world.GetAliveEntities().empty()) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.34f, 129.0f, 0, 255), "Right click here!");
        }

        for (EntityID entityId : world.GetAliveEntities()) {
            if (entityId == editorState.renameState.entityId) {
                ImGui::SetKeyboardFocusHere();
                ImGui::Indent();

                ImGui::SetKeyboardFocusHere();
                if (ImGui::InputText("##label", &editorState.renameState.nameBuffer,
                                     ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll |
                                         ImGuiInputTextFlags_CharsNoBlank)) {
                    editorState.renameState.Finish(world);
                    editorState.selection.Single(entityId);
                }
                ImGui::Unindent();
            }

            if (entityId != editorState.renameState.entityId) {
                bool selected = editorState.selection.Contains(entityId);
                DrawIcon(selected ? icons.entitySel : icons.entity, iconSz);

                ImGui::PushID((int)entityId);
                std::string entityName = world.HasComponent<NameComponent>(entityId)
                                             ? world.GetComponent<NameComponent>(entityId).name
                                             : "Entity_" + std::to_string(entityId);
                bool open = ImGui::TreeNodeEx(
                    entityName.c_str(), (selected ? ImGuiTreeNodeFlags_Selected : 0) |
                                            ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);
                ImGui::PopID();
                if (selected) {
                    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 40);
                    ImGui::Text("%i", entityId);
                }
                if (ImGui::IsItemClicked()) {
                    if (io.KeyCtrl)
                        editorState.selection.Toggle(entityId);
                    else
                        editorState.selection.Single(entityId);

                    if (editorState.selection.Contains(entityId)) {
                        inspectorState.dirty = true;
                        inspectorState.visible = true;
                    }
                }

                if (open) {
                    ImGui::Separator();

                    bool hasComponents = world.HasAnyComponent(entityId);
                    if (hasComponents) {
                        ImGui::Text("Components:");
                        ImGui::Indent();
                        if (world.HasComponent<ModelComponent>(entityId)) {
                            ImGui::BulletText("ModelComponent");
                        }
                        ImGui::Unindent();
                    }
                    ImGui::Unindent();
                    ImGui::Separator();
                    ImGui::TreePop();
                }
            }
        }

        ImGui::TreePop();
        if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered()) {
            editorState.selection.Clear();
        }
    }

    ImGui::End();
}
