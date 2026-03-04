#include "HierarchyPanel.h"

#include <Dodo.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void HierarchyPanel::Draw(EditorState& state, InspectorState& inspector)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Begin("Hierarchy");

    if (ImGui::BeginPopupContextWindow("Right-Click Hierarchy", ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::MenuItem("Create New")) {
            EntityID newEntity = state.scene->GetWorld().CreateEntity();
            state.renameState.Begin(state.scene->GetWorld(), newEntity);
        }
        ImGui::EndPopup();
    }

    ImGui::ColorButton("##EntitiesIcon", ImColor(226, 189, 0), ImGuiColorEditFlags_NoTooltip);
    ImGui::SameLine();

    if (ImGui::TreeNodeEx("Entities", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& world = state.scene->GetWorld();
        if (world.GetAliveEntities().empty()) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.34f, 129.0f, 0, 255), "Right click here!");
        }

        for (EntityID entityId : world.GetAliveEntities()) {
            if (entityId == state.renameState.entityId) { // Currently renaming this entity
                ImGui::SetKeyboardFocusHere();
                ImGui::Indent();

                ImGui::SetKeyboardFocusHere();
                if (ImGui::InputText("##label", &state.renameState.nameBuffer,
                                     ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll |
                                         ImGuiInputTextFlags_CharsNoBlank)) {
                    state.renameState.Finish(world);
                    state.selection.Single(entityId);
                }
                ImGui::Unindent();
            }

            if (entityId != state.renameState.entityId) {
                std::string colorButtonId = "##EntityIcon" + std::to_string(entityId);
                ImGui::ColorButton(colorButtonId.c_str(), ImColor(120, 50, 0), ImGuiColorEditFlags_NoTooltip);
                ImGui::SameLine();
                ImGui::PushID((int)entityId);
                std::string entityName = world.HasComponent<NameComponent>(entityId)
                                             ? world.GetComponent<NameComponent>(entityId).name
                                             : "Entity_" + std::to_string(entityId);
                bool open = ImGui::TreeNodeEx(
                    entityName.c_str(), (state.selection.Contains(entityId) ? ImGuiTreeNodeFlags_Selected : 0) |
                                            ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);
                ImGui::PopID();
                if (state.selection.Contains(entityId)) {
                    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 40); // Move text to right side
                    ImGui::Text("%i", entityId);
                }
                if (ImGui::IsItemClicked()) {
                    if (io.KeyCtrl)
                        state.selection.Toggle(entityId);
                    else
                        state.selection.Single(entityId);

                    if (state.selection.Contains(entityId)) {
                        inspector.dirty = true;
                        inspector.visible = true;
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
            state.selection.Clear();
        }
    }

    ImGui::End();
}