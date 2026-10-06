#include "AssetBrowserPanel.h"

#include "EditorTheme.h"
#include "EditorWidgets.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <imgui.h>
#include <string>
#include <vector>

namespace fs = std::filesystem;

EditorIcon AssetBrowserPanel::GetAssetIcon(const fs::path& path, bool isDir, ImVec4& tint)
{
    if (isDir) {
        tint = ImVec4(0.878f, 0.706f, 0.345f, 1.0f);
        return EditorIcon::Folder;
    }

    std::string ext = path.extension().string();
    for (auto& c : ext)
        c = (char)tolower((unsigned char)c);

    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".hdr" || ext == ".dds") {
        tint = ImVec4(0.722f, 0.537f, 0.902f, 1.0f);
        return EditorIcon::Texture;
    }
    if (ext == ".obj" || ext == ".gltf" || ext == ".glb" || ext == ".fbx") {
        tint = ImVec4(0.325f, 0.769f, 0.741f, 1.0f);
        return EditorIcon::Model;
    }
    if (ext == ".slang" || ext == ".glsl" || ext == ".hlsl") {
        tint = ImVec4(0.561f, 0.800f, 0.380f, 1.0f);
        return EditorIcon::Shader;
    }
    if (ext == ".das") {
        tint = EditorTheme::AccentHover;
        return EditorIcon::Scene;
    }
    tint = EditorTheme::TextDim;
    return EditorIcon::File;
}

void AssetBrowserPanel::Draw(AssetBrowserState& state, const EditorIcons& icons)
{
    if (!state.visible) return;

    if (!ImGui::Begin(state.name.c_str(), &state.visible)) {
        ImGui::End();
        return;
    }

    if (state.projectRoot.empty()) {
        EditorWidgets::EmptyState("No project open. Use File > Open > Project.");
        ImGui::End();
        return;
    }

    DrawToolbar(state, icons);
    ImGui::Dummy(ImVec2(0.0f, 2.0f));
    DrawGrid(state, icons);

    ImGui::End();
}

void AssetBrowserPanel::DrawToolbar(AssetBrowserState& state, const EditorIcons& icons)
{
    std::vector<fs::path> segments;
    fs::path dir = state.currentDir;
    while (true) {
        segments.push_back(dir);
        if (dir == state.projectRoot) break;
        fs::path parent = dir.parent_path();
        if (parent == dir) break;
        dir = parent;
    }
    std::reverse(segments.begin(), segments.end());

    const ImGuiStyle& style = ImGui::GetStyle();
    const float startX = ImGui::GetCursorPosX();
    const float fullWidth = ImGui::GetContentRegionAvail().x;

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, style.FramePadding.y));
    for (size_t i = 0; i < segments.size(); i++) {
        const bool last = i + 1 == segments.size();
        std::string label = segments[i].filename().string();

        ImGui::PushID((int)i);
        ImGui::PushStyleColor(ImGuiCol_Text, last ? EditorTheme::Text : EditorTheme::TextDim);
        if (ImGui::Button(label.c_str())) state.currentDir = segments[i];
        ImGui::PopStyleColor();
        ImGui::PopID();

        if (!last) {
            ImGui::SameLine(0.0f, 2.0f);
            ImGui::AlignTextToFramePadding();
            ImGui::TextDisabled("/");
            ImGui::SameLine(0.0f, 2.0f);
        }
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    // Search and tile size are right aligned, and dropped when the panel is too narrow to fit them.
    const float sliderWidth = 90.0f;
    const float searchWidth = 180.0f;
    const float rightWidth = searchWidth + sliderWidth + style.ItemSpacing.x;
    ImGui::SameLine();
    if (startX + fullWidth - rightWidth > ImGui::GetCursorPosX()) {
        ImGui::SetCursorPosX(startX + fullWidth - rightWidth);
        EditorWidgets::SearchField("##filter", state.filter, icons.Get(EditorIcon::Search), searchWidth);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(sliderWidth);
        ImGui::SliderFloat("##tileSize", &state.tileSize, 40.0f, 128.0f, "");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) ImGui::SetTooltip("Tile size");
    } else {
        ImGui::NewLine();
    }
}

void AssetBrowserPanel::DrawGrid(AssetBrowserState& state, const EditorIcons& icons)
{
    constexpr float tilePad = 8.0f;
    const float tileW = state.tileSize + tilePad * 2.0f;
    const float tileH = state.tileSize + ImGui::GetTextLineHeight() + tilePad * 3.0f;
    const float spacing = 4.0f;

    ImGui::BeginChild("##AssetGrid", ImVec2(0.0f, 0.0f), false);

    const float availW = ImGui::GetContentRegionAvail().x;
    const int maxCols = std::max(1, (int)((availW + spacing) / (tileW + spacing)));

    std::error_code ec;
    std::vector<fs::directory_entry> dirs, files;
    bool empty = true;
    for (const auto& entry : fs::directory_iterator(state.currentDir, ec)) {
        empty = false;
        if (!EditorWidgets::MatchesFilter(entry.path().filename().string(), state.filter)) continue;

        if (entry.is_directory(ec))
            dirs.push_back(entry);
        else
            files.push_back(entry);
    }

    auto byName = [](const fs::directory_entry& a, const fs::directory_entry& b) {
        return a.path().filename() < b.path().filename();
    };
    std::sort(dirs.begin(), dirs.end(), byName);
    std::sort(files.begin(), files.end(), byName);

    if (dirs.empty() && files.empty()) {
        EditorWidgets::EmptyState(empty ? "This folder is empty." : "No asset matches the search.");
        ImGui::EndChild();
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, spacing));
    int col = 0;
    auto drawEntry = [&](const fs::directory_entry& entry) {
        if (col > 0) ImGui::SameLine();
        DrawTile(entry, tileW, tileH, state, icons);
        col = (col + 1) % maxCols;
    };

    for (const auto& d : dirs)
        drawEntry(d);
    for (const auto& f : files)
        drawEntry(f);
    ImGui::PopStyleVar();

    ImGui::EndChild();
}

void AssetBrowserPanel::DrawTile(const fs::directory_entry& entry, float tileW, float tileH, AssetBrowserState& state,
                                 const EditorIcons& icons)
{
    constexpr float tilePad = 8.0f;
    std::error_code ec;
    const fs::path& path = entry.path();
    const bool isDir = entry.is_directory(ec);
    const std::string name = path.filename().string();
    const bool isSelected = state.selectedPath == path;

    ImGui::PushID(path.generic_string().c_str());

    const ImVec2 tileMin = ImGui::GetCursorScreenPos();
    const ImVec2 tileMax = {tileMin.x + tileW, tileMin.y + tileH};

    ImGui::InvisibleButton("##tile", ImVec2(tileW, tileH));
    const bool hovered = ImGui::IsItemHovered();

    if (ImGui::IsItemClicked()) state.selectedPath = path;
    if (hovered && ImGui::IsMouseDoubleClicked(0) && isDir) {
        state.currentDir = path;
        state.filter.clear();
    }
    if (hovered && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) ImGui::SetTooltip("%s", name.c_str());

    ImDrawList* dl = ImGui::GetWindowDrawList();

    if (isSelected) {
        dl->AddRectFilled(tileMin, tileMax, ImGui::GetColorU32(EditorTheme::Selection), 6.0f);
        dl->AddRect(tileMin, tileMax, ImGui::GetColorU32(EditorTheme::Accent), 6.0f);
    } else if (hovered) {
        dl->AddRectFilled(tileMin, tileMax, ImGui::GetColorU32(EditorTheme::Hover), 6.0f);
    }

    const float iconX = std::floor(tileMin.x + (tileW - state.tileSize) * 0.5f);
    const float iconY = tileMin.y + tilePad;

    ImVec4 tint;
    if (void* texID = icons.Get(GetAssetIcon(path, isDir, tint))) {
        dl->AddImage(texID, ImVec2(iconX, iconY), ImVec2(iconX + state.tileSize, iconY + state.tileSize),
                     ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImGui::GetColorU32(tint));
    }

    const float labelY = iconY + state.tileSize + tilePad;
    EditorWidgets::TextEllipsis(dl, ImVec2(tileMin.x + tilePad, labelY), tileW - tilePad * 2.0f,
                                ImGui::GetColorU32(EditorTheme::Text), name);

    ImGui::PopID();
}
