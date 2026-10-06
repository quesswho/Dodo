#pragma once

#include <Dodo.h>

#include <array>
#include <vector>

enum class EditorIcon {
    // Interface icons, drawn at text size
    Entity,
    Move,
    Rotate,
    Scale,
    Local,
    World,
    Search,
    Add,
    // Asset icons, drawn at tile size
    Folder,
    Texture,
    Model,
    Shader,
    Scene,
    File,
    Count
};

/**
 * Icon set of the editor, rasterized from the SVG files in res/editor/icons.
 *
 * Every icon is stored as a white alpha mask, so the color is chosen at draw time through the ImGui tint.
 */
class EditorIcons {
  public:
    void Load(Dodo::RenderAPI& api);

    /**
     * @return ImGui texture of the icon, or null if its file is missing. Callers are expected to fall back to text.
     */
    void* Get(EditorIcon icon) const { return m_ImGuiIDs[(size_t)icon]; }

  private:
    /**
     * @return Tightly packed RGBA pixels of size * size, or an empty vector if the file could not be parsed.
     */
    static std::vector<unsigned char> RasterizeSVG(const char* path, int size);

    std::array<Ref<Dodo::Texture>, (size_t)EditorIcon::Count> m_Textures;
    std::array<void*, (size_t)EditorIcon::Count> m_ImGuiIDs = {};
};
