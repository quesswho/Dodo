#include "AssetBrowserPanel.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void AssetBrowserPanel::Draw(AssetBrowserState& state)
{
    ImGui::Begin(state.name.c_str());

    ImGui::End();
}