#include "AssetBrowserPanel.h"

#include <filesystem>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace fs = std::filesystem;

void AssetBrowserPanel::Draw(AssetBrowserState& state)
{
    ImGui::Begin(state.name.c_str());

    if (state.projectRoot.empty()) {
        ImGui::TextDisabled("No project open. Use File > New > Project or File > Open > Project.");
        ImGui::End();
        return;
    }

    // Breadcrumb: show path relative to project root
    fs::path rel = fs::relative(state.currentDir, state.projectRoot.parent_path());
    ImGui::TextUnformatted(rel.generic_string().c_str());
    ImGui::Separator();

    // Navigate up (but not above assets root)
    if (state.currentDir != state.projectRoot) {
        if (ImGui::Selectable("../", false, ImGuiSelectableFlags_AllowDoubleClick)) {
            if (ImGui::IsMouseDoubleClicked(0))
                state.currentDir = state.currentDir.parent_path();
        }
    }

    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(state.currentDir, ec)) {
        const fs::path& p = entry.path();
        std::string label = p.filename().string();

        if (entry.is_directory(ec)) {
            label += "/";
            if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                if (ImGui::IsMouseDoubleClicked(0))
                    state.currentDir = p;
            }
        } else {
            ImGui::Selectable(label.c_str());
        }
    }

    ImGui::End();
}
