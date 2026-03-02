#include "HierarchyPanel.h"

#include <Dodo.h>
#include <imgui.h>

void HierarchyPanel::Draw(EditorState& state, InspectorState& inspector)
{
    ImGuiIO &io = ImGui::GetIO();
    static bool s_ClickHandled = false;
    static uint s_RenamingId = -1; // 4 294 967 295
    ImGui::Begin("Hierarchy");

    if (ImGui::BeginPopupContextWindow("Right-Click Hierarchy", ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::MenuItem("Create New"))
        {
            s_RenamingId = state.scene->GetWorld().CreateEntity();
            state.selection.Single(s_RenamingId);
            s_ClickHandled = true;
        }
        ImGui::EndPopup();
    }

    static char s_RenameableHierarchy[32] = "Unnamed";

    ImGui::ColorButton("##EntitiesIcon", ImColor(226, 189, 0), ImGuiColorEditFlags_NoTooltip);
    ImGui::SameLine();

    if (ImGui::TreeNodeEx("Entities", ImGuiTreeNodeFlags_DefaultOpen))
    {
        World &world = state.scene->GetWorld();
        if (world.GetAliveEntities().empty())
        {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.34f, 129.0f, 0, 255), "Right click here!");
        }
        for (EntityID entityId : world.GetAliveEntities())
        {
            if (entityId != s_RenamingId)
            {
                std::string colorButtonId = "##EntityIcon" + std::to_string(entityId);
                ImGui::ColorButton(colorButtonId.c_str(), ImColor(120, 50, 0), ImGuiColorEditFlags_NoTooltip);
                ImGui::SameLine();
                ImGui::PushID((int)entityId);
                std::string entityName = world.HasComponent<NameComponent>(entityId)
                                                ? world.GetComponent<NameComponent>(entityId).m_Name
                                                : "Entity_" + std::to_string(entityId);
                bool open = ImGui::TreeNodeEx(
                    entityName.c_str(),
                    (state.selection.Contains(entityId) ? ImGuiTreeNodeFlags_Selected : 0) |
                        ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);
                ImGui::PopID();
                if (state.selection.Contains(entityId))
                {
                    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 40); // Move text to right side
                    ImGui::Text("%i", entityId);
                }
                if (ImGui::IsItemClicked())
                {
                    if (io.KeyCtrl)
                        state.selection.Toggle(entityId);
                    else
                        state.selection.Single(entityId);

                    if (world.HasComponent<NameComponent>(entityId))
                    {
                        inspector.nameBuffer = world.GetComponent<NameComponent>(entityId).m_Name;
                    }
                    s_ClickHandled = true;

                    if (state.selection.Contains(entityId))
                    {
                        inspector.dirty = true;
                        inspector.visible = true;
                    }
                }

                if (open)
                {
                    ImGui::Separator();

                    // Check what components this entity has
                    bool hasComponents = world.HasAnyComponent(entityId);
                    if (hasComponents)
                    {
                        ImGui::Text("Components:");
                        ImGui::Indent();
                        if (world.HasComponent<ModelComponent>(entityId))
                        {
                            ImGui::BulletText("ModelComponent");
                        }
                        ImGui::Unindent();
                    }
                    ImGui::Unindent();
                    ImGui::Separator();
                    ImGui::TreePop();
                }
            } else
            {
                ImGui::SetKeyboardFocusHere(0);
                ImGui::Indent();
                if (ImGui::InputText(std::to_string(entityId).c_str(), s_RenameableHierarchy,
                                        IM_ARRAYSIZE(s_RenameableHierarchy),
                                        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll |
                                            ImGuiInputTextFlags_CharsNoBlank))
                {
                    if (s_RenameableHierarchy == nullptr || s_RenameableHierarchy[0] == '\0')
                        strncpy(s_RenameableHierarchy, "Unnamed", 256);

                    if (world.HasComponent<NameComponent>(entityId))
                    {
                        world.GetComponent<NameComponent>(entityId).m_Name = std::string(s_RenameableHierarchy);
                    } else
                    {
                        world.AddComponent<NameComponent>(entityId,
                                                            NameComponent{std::string(s_RenameableHierarchy)});
                    }
                    inspector.nameBuffer = std::string(s_RenameableHierarchy);
                    strncpy(s_RenameableHierarchy, "Unnamed", 256);
                    s_RenamingId = -1;
                    if (io.KeyCtrl)
                        state.selection.Toggle(entityId);
                    else
                        state.selection.Single(entityId);

                    inspector.dirty = true;
                }
                ImGui::Unindent();
            }
        }

        ImGui::TreePop();
        if (!s_ClickHandled && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered())
        {
            state.selection.Clear();
        }

        s_ClickHandled = false;
    }

    ImGui::End();
}