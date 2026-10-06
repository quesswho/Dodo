#pragma once
#include <filesystem>
#include <string>

struct AssetBrowserState {
    std::string name = "Asset Browser";
    bool visible = false;

    std::filesystem::path projectRoot;
    std::filesystem::path currentDir;
    std::filesystem::path selectedPath;

    std::string filter;
    float tileSize = 64.0f;
};
