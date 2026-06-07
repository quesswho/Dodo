#include "AssetBrowserPanel.h"

#include "EditorIcons.h"

#include <filesystem>
#include <imgui.h>
#include <algorithm>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static void* GetAssetIcon(const EditorIconSet& icons, const fs::path& path, bool isDir)
{
    if (isDir)
        return icons.folder;
    std::string ext = path.extension().string();
    for (auto& c : ext) c = (char)tolower((unsigned char)c);
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".hdr" || ext == ".dds")
        return icons.texture;
    if (ext == ".obj" || ext == ".gltf" || ext == ".glb" || ext == ".fbx")
        return icons.model;
    if (ext == ".slang" || ext == ".glsl" || ext == ".hlsl")
        return icons.shader;
    if (ext == ".das")
        return icons.scene;
    return icons.file;
}

void AssetBrowserPanel::Draw(AssetBrowserState& state)
{
    if (!state.visible)
        return;

    if (!ImGui::Begin(state.name.c_str(), &state.visible)) {
        ImGui::End();
        return;
    }

    if (state.projectRoot.empty()) {
        ImGui::TextDisabled("No project open. Use File > Open > Project.");
        ImGui::End();
        return;
    }

    DrawBreadcrumb(state);
    ImGui::Separator();
    DrawGrid(state);

    ImGui::End();
}

void AssetBrowserPanel::DrawBreadcrumb(AssetBrowserState& state)
{
    std::vector<fs::path> segments;
    fs::path dir = state.currentDir;
    while (true) {
        segments.push_back(dir);
        if (dir == state.projectRoot)
            break;
        fs::path parent = dir.parent_path();
        if (parent == dir)
            break;
        dir = parent;
    }
    std::reverse(segments.begin(), segments.end());

    for (size_t i = 0; i < segments.size(); i++) {
        std::string label = segments[i].filename().string();
        if (ImGui::SmallButton(label.c_str()))
            state.currentDir = segments[i];
        if (i + 1 < segments.size()) {
            ImGui::SameLine(0.0f, 2.0f);
            ImGui::TextDisabled("/");
            ImGui::SameLine(0.0f, 2.0f);
        }
    }
}

void AssetBrowserPanel::DrawGrid(AssetBrowserState& state)
{
    constexpr float tilePad = 8.0f;
    const float tileW = state.tileSize + tilePad * 2.0f;
    const float tileH = state.tileSize + ImGui::GetTextLineHeight() + tilePad * 3.0f;

    const float availW  = ImGui::GetContentRegionAvail().x;
    const int   maxCols = std::max(1, (int)(availW / tileW));

    ImGui::BeginChild("##AssetGrid", ImVec2(0.0f, 0.0f), false);

    std::error_code ec;
    std::vector<fs::directory_entry> dirs, files;
    for (const auto& entry : fs::directory_iterator(state.currentDir, ec)) {
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

    int col = 0;
    auto drawEntry = [&](const fs::directory_entry& entry) {
        if (col > 0)
            ImGui::SameLine();
        DrawTile(entry, tileW, tileH, state);
        col = (col + 1) % maxCols;
    };

    for (const auto& d : dirs)  drawEntry(d);
    for (const auto& f : files) drawEntry(f);

    ImGui::EndChild();
}

void AssetBrowserPanel::DrawTile(const fs::directory_entry& entry, float tileW, float tileH,
                                  AssetBrowserState& state)
{
    constexpr float tilePad = 8.0f;
    std::error_code ec;
    const fs::path& path  = entry.path();
    const bool      isDir = entry.is_directory(ec);
    const std::string name = path.filename().string();
    const bool isSelected  = state.selectedPath == path;

    ImGui::PushID(path.generic_string().c_str());

    const ImVec2 tileMin = ImGui::GetCursorScreenPos();
    const ImVec2 tileMax = { tileMin.x + tileW, tileMin.y + tileH };

    ImGui::InvisibleButton("##tile", ImVec2(tileW, tileH));
    const bool hovered = ImGui::IsItemHovered();

    if (ImGui::IsItemClicked())
        state.selectedPath = path;
    if (hovered && ImGui::IsMouseDoubleClicked(0) && isDir)
        state.currentDir = path;

    ImDrawList* dl = ImGui::GetWindowDrawList();

    if (isSelected)
        dl->AddRectFilled(tileMin, tileMax, IM_COL32(0, 120, 215, 100), 4.0f);
    else if (hovered)
        dl->AddRectFilled(tileMin, tileMax, IM_COL32(255, 255, 255, 18), 4.0f);

    const float iconX = tileMin.x + (tileW - state.tileSize) * 0.5f;
    const float iconY = tileMin.y + tilePad;

    if (state.icons && state.icons->ready) {
        void* texID = GetAssetIcon(*state.icons, path, isDir);
        if (texID) {
            dl->AddImage(texID,
                         ImVec2(iconX, iconY),
                         ImVec2(iconX + state.tileSize, iconY + state.tileSize));
        }
    }

    const float labelY = iconY + state.tileSize + tilePad;
    const ImVec2 labelSize = ImGui::CalcTextSize(name.c_str());
    const float maxLabelW  = tileW - tilePad * 2.0f;
    const float labelX     = tileMin.x + (tileW - std::min(labelSize.x, maxLabelW)) * 0.5f;

    dl->PushClipRect(
        ImVec2(tileMin.x + tilePad, tileMin.y),
        ImVec2(tileMax.x - tilePad, tileMax.y), true);
    dl->AddText(ImVec2(labelX, labelY), IM_COL32(210, 210, 210, 255), name.c_str());
    dl->PopClipRect();

    ImGui::PopID();
}
