#include "HierarchyPanel.h"

#include <Dodo.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void HierarchyPanel::Draw(EditorState& editorState, InspectorState& inspectorState, HierarchyState& state)
{
    if (!ImGui::Begin(state.name.c_str(), &state.visible)) {
        ImGui::End();
        return;
    }
    
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::BeginPopupContextWindow("Right-Click Hierarchy", ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::MenuItem("Create New")) {
            EntityID newEntity = editorState.scene->GetWorld().CreateEntity();
            editorState.renameState.Begin(editorState.scene->GetWorld(), newEntity);
        }
        ImGui::EndPopup();
    }

    ImGui::ColorButton("##EntitiesIcon", ImColor(226, 189, 0), ImGuiColorEditFlags_NoTooltip);
    ImGui::SameLine();

    if (ImGui::TreeNodeEx("Entities", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& world = editorState.scene->GetWorld();
        if (world.GetAliveEntities().empty()) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.34f, 129.0f, 0, 255), "Right click here!");
        }

        for (EntityID entityId : world.GetAliveEntities()) {
            if (entityId == editorState.renameState.entityId) { // Currently renaming this entity
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
                std::string colorButtonId = "##EntityIcon" + std::to_string(entityId);
                ImGui::ColorButton(colorButtonId.c_str(), ImColor(120, 50, 0), ImGuiColorEditFlags_NoTooltip);
                ImGui::SameLine();
                ImGui::PushID((int)entityId);
                std::string entityName = world.HasComponent<NameComponent>(entityId)
                                             ? world.GetComponent<NameComponent>(entityId).name
                                             : "Entity_" + std::to_string(entityId);
                bool open = ImGui::TreeNodeEx(
                    entityName.c_str(), (editorState.selection.Contains(entityId) ? ImGuiTreeNodeFlags_Selected : 0) |
                                            ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);
                ImGui::PopID();
                if (editorState.selection.Contains(entityId)) {
                    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 40); // Move text to right side
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

                    // Check what components this entity has
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