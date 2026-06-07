#pragma once

#include "PanelStates/AssetBrowserState.h"

#include <filesystem>

class AssetBrowserPanel {
  public:
    void Draw(AssetBrowserState& state);

  private:
    void DrawBreadcrumb(AssetBrowserState& state);
    void DrawGrid(AssetBrowserState& state);
    void DrawTile(const std::filesystem::directory_entry& entry, float tileW, float tileH,
                  AssetBrowserState& state);
};
