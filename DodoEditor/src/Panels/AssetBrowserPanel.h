#pragma once

#include "EditorIcons.h"
#include "PanelStates/AssetBrowserState.h"

#include <filesystem>
#include <imgui.h>

class AssetBrowserPanel {
  public:
    void Draw(AssetBrowserState& state, const EditorIcons& icons);

  private:
    /**
     * Draws the path of the current directory as clickable segments, followed by the search field and the
     * tile size slider on the right.
     */
    void DrawToolbar(AssetBrowserState& state, const EditorIcons& icons);
    void DrawGrid(AssetBrowserState& state, const EditorIcons& icons);
    void DrawTile(const std::filesystem::directory_entry& entry, float tileW, float tileH, AssetBrowserState& state,
                  const EditorIcons& icons);

    /**
     * Picks the icon and its tint from the kind of asset the path points to.
     */
    static EditorIcon GetAssetIcon(const std::filesystem::path& path, bool isDir, ImVec4& tint);
};
