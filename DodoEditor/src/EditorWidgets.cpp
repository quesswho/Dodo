#include "EditorWidgets.h"

#include "EditorTheme.h"

#include <algorithm>
#include <cctype>
#include <cmath>

#include <misc/cpp/imgui_stdlib.h>

bool EditorWidgets::ToolButton(const char* label, void* icon, const char* tooltip, bool selected)
{
    const ImVec4 transparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, selected ? EditorTheme::Accent : transparent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, selected ? EditorTheme::AccentHover : EditorTheme::RaisedHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, selected ? EditorTheme::AccentActive : EditorTheme::RaisedActive);

    bool clicked = false;
    if (icon) {
        const float size = ImGui::GetFrameHeight();
        const float iconSize = 16.0f;

        ImGui::PushID(label);
        clicked = ImGui::Button("##tool", ImVec2(size, size));
        ImGui::PopID();

        const ImVec2 min = ImGui::GetItemRectMin();
        const float offset = std::floor((size - iconSize) * 0.5f);
        const ImVec2 iconMin = ImVec2(min.x + offset, min.y + offset);
        const ImVec4 tint = selected ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : EditorTheme::Text;
        ImGui::GetWindowDrawList()->AddImage(icon, iconMin, ImVec2(iconMin.x + iconSize, iconMin.y + iconSize),
                                             ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImGui::GetColorU32(tint));
    } else {
        clicked = ImGui::Button(label);
    }
    ImGui::PopStyleColor(3);

    if (tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) ImGui::SetTooltip("%s", tooltip);
    return clicked;
}

void EditorWidgets::ToolSeparator()
{
    const float height = ImGui::GetFrameHeight();
    ImGui::Dummy(ImVec2(7.0f, height));

    const ImVec2 min = ImGui::GetItemRectMin();
    const float x = min.x + 3.0f;
    ImGui::GetWindowDrawList()->AddLine(ImVec2(x, min.y + 4.0f), ImVec2(x, min.y + height - 4.0f),
                                        ImGui::GetColorU32(EditorTheme::Border));
}

bool EditorWidgets::SearchField(const char* id, std::string& text, void* icon, float width)
{
    const ImGuiStyle& style = ImGui::GetStyle();
    const float height = ImGui::GetFrameHeight();
    const float iconSize = 14.0f;
    const ImVec2 pos = ImGui::GetCursorScreenPos();

    // The frame is drawn by hand, so the icon can sit inside of it in front of the text.
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), ImGui::GetColorU32(ImGuiCol_FrameBg),
                            style.FrameRounding);

    float inset = 0.0f;
    if (icon) {
        const ImVec2 iconMin = ImVec2(pos.x + style.FramePadding.x, pos.y + std::floor((height - iconSize) * 0.5f));
        drawList->AddImage(icon, iconMin, ImVec2(iconMin.x + iconSize, iconMin.y + iconSize), ImVec2(0.0f, 0.0f),
                           ImVec2(1.0f, 1.0f), ImGui::GetColorU32(EditorTheme::TextDim));
        inset = style.FramePadding.x + iconSize - 2.0f;
    }

    const ImVec4 transparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, transparent);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, transparent);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, transparent);
    ImGui::SetCursorScreenPos(ImVec2(pos.x + inset, pos.y));
    ImGui::SetNextItemWidth(std::max(width - inset, 1.0f));
    const bool changed = ImGui::InputTextWithHint(id, "Search", &text);
    ImGui::PopStyleColor(3);

    return changed;
}

bool EditorWidgets::MatchesFilter(const std::string& name, const std::string& filter)
{
    if (filter.empty()) return true;

    auto equalIgnoringCase = [](char a, char b) {
        return std::tolower((unsigned char)a) == std::tolower((unsigned char)b);
    };
    return std::search(name.begin(), name.end(), filter.begin(), filter.end(), equalIgnoringCase) != name.end();
}

bool EditorWidgets::Vec3Control(const char* id, Dodo::Math::Vec3& value, float speed, const char* format)
{
    const ImGuiStyle& style = ImGui::GetStyle();
    const ImVec4 axisColors[3] = {EditorTheme::AxisX, EditorTheme::AxisY, EditorTheme::AxisZ};
    float* components[3] = {&value.x, &value.y, &value.z};

    const float spacing = style.ItemInnerSpacing.x;
    const float width = std::max((ImGui::GetContentRegionAvail().x - spacing * 2.0f) / 3.0f, 1.0f);

    bool changed = false;
    ImGui::PushID(id);
    for (int i = 0; i < 3; i++) {
        if (i > 0) ImGui::SameLine(0.0f, spacing);

        ImGui::PushID(i);
        ImGui::SetNextItemWidth(width);
        changed |= ImGui::DragFloat("##axis", components[i], speed, 0.0f, 0.0f, format);
        ImGui::PopID();

        // Colored strip on the left edge of the field tells the axes apart.
        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 max = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRectFilled(min, ImVec2(min.x + 3.0f, max.y), ImGui::GetColorU32(axisColors[i]),
                                                  style.FrameRounding, ImDrawFlags_RoundCornersLeft);
    }
    ImGui::PopID();

    return changed;
}

bool EditorWidgets::SectionHeader(const char* label)
{
    // Headers share their colors with selected rows by default, a section is not a selection.
    ImGui::PushStyleColor(ImGuiCol_Header, EditorTheme::Raised);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::RaisedHover);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, EditorTheme::RaisedActive);
    const bool open = ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen);
    ImGui::PopStyleColor(3);
    return open;
}

bool EditorWidgets::BeginProperties(const char* id)
{
    if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_NoSavedSettings)) return false;

    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 76.0f);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
    return true;
}

void EditorWidgets::Property(const char* label)
{
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(EditorTheme::TextDim, "%s", label);
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
}

void EditorWidgets::EndProperties()
{
    ImGui::EndTable();
}

void EditorWidgets::EmptyState(const char* text)
{
    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const float wrapWidth = std::max(avail.x - 24.0f, 1.0f);
    const ImVec2 size = ImGui::CalcTextSize(text, nullptr, false, wrapWidth);

    const ImVec2 cursor = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(cursor.x + std::max(std::floor((avail.x - size.x) * 0.5f), 0.0f),
                               cursor.y + std::max(std::floor((avail.y - size.y) * 0.4f), 0.0f)));

    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::TextDim);
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + wrapWidth);
    ImGui::TextUnformatted(text);
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
}

void EditorWidgets::TextEllipsis(ImDrawList* drawList, const ImVec2& pos, float width, ImU32 color,
                                 const std::string& text)
{
    std::string shown = text;
    float shownWidth = ImGui::CalcTextSize(shown.c_str()).x;

    if (shownWidth > width) {
        std::string cut = text;
        while (!cut.empty()) {
            // Remove one whole UTF-8 character: its continuation bytes first, then the leading byte.
            while (cut.size() > 1 && ((unsigned char)cut.back() & 0xC0) == 0x80)
                cut.pop_back();
            cut.pop_back();

            shown = cut + "...";
            shownWidth = ImGui::CalcTextSize(shown.c_str()).x;
            if (shownWidth <= width) break;
        }
    }

    const float x = pos.x + std::max(std::floor((width - shownWidth) * 0.5f), 0.0f);
    drawList->AddText(ImVec2(x, pos.y), color, shown.c_str());
}
