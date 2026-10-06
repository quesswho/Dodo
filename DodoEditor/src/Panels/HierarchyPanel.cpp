#include "HierarchyPanel.h"

#include "EditorTheme.h"
#include "EditorWidgets.h"

#include <Dodo.h>
#include <algorithm>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

std::string HierarchyPanel::GetEntityName(EditorWorld& world, EntityID entityId)
{
    return world.HasComponent<NameComponent>(entityId) ? world.GetComponent<NameComponent>(entityId).name
                                                       : "Entity_" + std::to_string(entityId);
}

void HierarchyPanel::CreateEntity(EditorState& editorState)
{
    EntityID newEntity = editorState.scene->GetWorld().CreateEntity();
    editorState.renameState.Begin(editorState.scene->GetWorld(), newEntity);
}

void HierarchyPanel::DeleteSelection(EditorState& editorState, InspectorState& inspectorState)
{
    auto& world = editorState.scene->GetWorld();
    for (EntityID id : editorState.selection.entities) {
        if (id == editorState.renameState.entityId) editorState.renameState.Cancel();
        world.DeleteEntity(id);
    }
    editorState.selection.Clear();
    inspectorState.dirty = false;
}

void HierarchyPanel::Draw(EditorState& editorState, InspectorState& inspectorState, HierarchyState& state,
                          const EditorIcons& icons)
{
    if (!state.visible) return;

    if (!ImGui::Begin(state.name.c_str(), &state.visible)) {
        ImGui::End();
        return;
    }

    auto& world = editorState.scene->GetWorld();
    RenameState& renameState = editorState.renameState;

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Delete) &&
        !renameState.isActive() && !editorState.selection.Empty()) {
        DeleteSelection(editorState, inspectorState);
    }

    // Search field and the create button stay in place while the list below scrolls.
    const float buttonWidth = ImGui::GetFrameHeight();
    const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
    EditorWidgets::SearchField("##filter", state.filter, icons.Get(EditorIcon::Search),
                               ImGui::GetContentRegionAvail().x - buttonWidth - spacing);
    ImGui::SameLine(0.0f, spacing);
    if (EditorWidgets::ToolButton("+", icons.Get(EditorIcon::Add), "Create entity")) CreateEntity(editorState);

    ImGui::BeginChild("##entities");

    if (ImGui::BeginPopupContextWindow("##hierarchyContext",
                                       ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        if (ImGui::MenuItem("Create Entity")) CreateEntity(editorState);
        ImGui::EndPopup();
    }

    // The world keeps its entities in a hash set, sort them so the list does not reorder itself.
    std::vector<EntityID> entities(world.GetAliveEntities().begin(), world.GetAliveEntities().end());
    std::sort(entities.begin(), entities.end());

    bool deleteRequested = false;
    int shown = 0;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 1.0f));
    for (EntityID entityId : entities) {
        const bool renaming = entityId == renameState.entityId;
        const std::string name = GetEntityName(world, entityId);
        if (!renaming && !EditorWidgets::MatchesFilter(name, state.filter)) continue;

        ImGui::PushID((int)entityId);
        if (renaming)
            DrawRenameRow(editorState, entityId);
        else
            deleteRequested |= DrawEntityRow(editorState, inspectorState, entityId, name, icons);
        ImGui::PopID();
        shown++;
    }
    ImGui::PopStyleVar();

    if (shown == 0) {
        EditorWidgets::EmptyState(entities.empty() ? "The scene is empty. Right click or press + to create an entity."
                                                   : "No entity matches the search.");
    }

    if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered()) {
        editorState.selection.Clear();
    }

    ImGui::EndChild();

    // Deleting is deferred to here, the rows above must not outlive their entities.
    if (deleteRequested) DeleteSelection(editorState, inspectorState);

    ImGui::End();
}

bool HierarchyPanel::DrawEntityRow(EditorState& editorState, InspectorState& inspectorState, EntityID entityId,
                                   const std::string& name, const EditorIcons& icons)
{
    const ImGuiStyle& style = ImGui::GetStyle();
    Selection& selection = editorState.selection;

    const float rowHeight = ImGui::GetFrameHeight();
    const ImVec2 rowMin = ImGui::GetCursorScreenPos();
    const ImVec2 rowMax = ImVec2(rowMin.x + ImGui::GetContentRegionAvail().x, rowMin.y + rowHeight);

    ImGui::InvisibleButton("##row", ImVec2(rowMax.x - rowMin.x, rowHeight));
    const bool hovered = ImGui::IsItemHovered();

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        if (ImGui::GetIO().KeyCtrl)
            selection.Toggle(entityId);
        else
            selection.Single(entityId);
    }
    // The context menu acts on the selection, so a right click on an unselected row selects it first.
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && !selection.Contains(entityId)) selection.Single(entityId);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        if (selection.Contains(entityId)) {
            inspectorState.dirty = true;
            inspectorState.visible = true;
        }
    }

    if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        editorState.renameState.Begin(editorState.scene->GetWorld(), entityId);

    bool deleteRequested = false;
    if (ImGui::BeginPopupContextItem("##entityContext")) {
        if (ImGui::MenuItem("Rename")) editorState.renameState.Begin(editorState.scene->GetWorld(), entityId);
        if (ImGui::MenuItem("Delete", "Del")) deleteRequested = true;
        ImGui::Separator();
        if (ImGui::MenuItem("Create Entity")) CreateEntity(editorState);
        ImGui::EndPopup();
    }

    const bool selected = selection.Contains(entityId);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    if (selected)
        drawList->AddRectFilled(rowMin, rowMax, ImGui::GetColorU32(EditorTheme::Selection), EditorTheme::Rounding);
    else if (hovered)
        drawList->AddRectFilled(rowMin, rowMax, ImGui::GetColorU32(EditorTheme::Hover), EditorTheme::Rounding);

    const float iconSize = 16.0f;
    float textX = rowMin.x + style.FramePadding.x;
    if (void* icon = icons.Get(EditorIcon::Entity)) {
        const ImVec2 iconMin = ImVec2(textX, rowMin.y + std::floor((rowHeight - iconSize) * 0.5f));
        drawList->AddImage(icon, iconMin, ImVec2(iconMin.x + iconSize, iconMin.y + iconSize), ImVec2(0.0f, 0.0f),
                           ImVec2(1.0f, 1.0f),
                           ImGui::GetColorU32(selected ? EditorTheme::AccentHover : EditorTheme::TextDim));
        textX += iconSize + style.ItemInnerSpacing.x;
    }

    // The entity id is only of interest for the rows the user is working with.
    float textMaxX = rowMax.x - style.FramePadding.x;
    if (selected || hovered) {
        const std::string id = std::to_string(entityId);
        textMaxX -= ImGui::CalcTextSize(id.c_str()).x;
        drawList->AddText(ImVec2(textMaxX, rowMin.y + style.FramePadding.y), ImGui::GetColorU32(EditorTheme::TextDim),
                          id.c_str());
        textMaxX -= style.ItemInnerSpacing.x;
    }

    drawList->PushClipRect(ImVec2(textX, rowMin.y), ImVec2(textMaxX, rowMax.y), true);
    drawList->AddText(ImVec2(textX, rowMin.y + style.FramePadding.y), ImGui::GetColorU32(EditorTheme::Text),
                      name.c_str());
    drawList->PopClipRect();

    return deleteRequested;
}

void HierarchyPanel::DrawRenameRow(EditorState& editorState, EntityID entityId)
{
    RenameState& renameState = editorState.renameState;
    auto& world = editorState.scene->GetWorld();

    const bool requestFocus = renameState.focusFrames > 0;
    if (requestFocus) {
        ImGui::SetKeyboardFocusHere();
        renameState.focusFrames--;
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    const bool confirmed = ImGui::InputText("##rename", &renameState.nameBuffer,
                                            ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll |
                                                ImGuiInputTextFlags_CharsNoBlank);

    // Clicking anywhere else also ends the rename and keeps what was typed.
    if (confirmed) {
        renameState.Finish(world);
        editorState.selection.Single(entityId);
    } else if (!requestFocus && ImGui::IsItemDeactivated()) {
        renameState.Finish(world);
    }
}
