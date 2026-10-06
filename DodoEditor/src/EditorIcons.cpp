#include "EditorIcons.h"

// Implementation is compiled by the nanosvg/nanosvgrast CMake targets.
// Include headers here without the implementation defines.
#include "nanosvg.h"
#include "nanosvgrast.h"

#include <algorithm>
#include <string>
#include <thread>

// Icons are rasterized larger than they are drawn and scaled down through their mipmaps, which keeps them sharp.
static constexpr int INTERFACE_ICON_SIZE = 32;
static constexpr int ASSET_ICON_SIZE = 256;

std::vector<unsigned char> EditorIcons::RasterizeSVG(const char* path, int size)
{
    NSVGimage* img = nsvgParseFromFile(path, "px", 96.0f);
    if (!img) return {};

    NSVGrasterizer* rast = nsvgCreateRasterizer();
    if (!rast) {
        nsvgDelete(img);
        return {};
    }

    std::vector<unsigned char> pixels(size * size * 4, 0);
    float maxDim = std::max(img->width, img->height);
    float scale = (maxDim > 0.0f) ? ((float)size / maxDim) : 1.0f;
    nsvgRasterize(rast, img, 0.0f, 0.0f, scale, pixels.data(), size, size, size * 4);

    nsvgDeleteRasterizer(rast);
    nsvgDelete(img);

    // Keep only the coverage: a white mask can be tinted to any color when it is drawn.
    for (size_t i = 0; i < pixels.size(); i += 4) {
        pixels[i] = 255;
        pixels[i + 1] = 255;
        pixels[i + 2] = 255;
    }

    return pixels;
}

void EditorIcons::Load(Dodo::RenderAPI& api)
{
    using namespace Dodo;

    struct IconSource {
        EditorIcon icon;
        const char* file;
        int size;
    };
    static constexpr IconSource sources[] = {
        {EditorIcon::Entity, "entity", INTERFACE_ICON_SIZE}, {EditorIcon::Move, "move", INTERFACE_ICON_SIZE},
        {EditorIcon::Rotate, "rotate", INTERFACE_ICON_SIZE}, {EditorIcon::Scale, "scale", INTERFACE_ICON_SIZE},
        {EditorIcon::Local, "local", INTERFACE_ICON_SIZE},   {EditorIcon::World, "world", INTERFACE_ICON_SIZE},
        {EditorIcon::Search, "search", INTERFACE_ICON_SIZE}, {EditorIcon::Add, "add", INTERFACE_ICON_SIZE},
        {EditorIcon::Folder, "folder", ASSET_ICON_SIZE},     {EditorIcon::Texture, "texture", ASSET_ICON_SIZE},
        {EditorIcon::Model, "model", ASSET_ICON_SIZE},       {EditorIcon::Shader, "shader", ASSET_ICON_SIZE},
        {EditorIcon::Scene, "scene", ASSET_ICON_SIZE},       {EditorIcon::File, "file", ASSET_ICON_SIZE},
    };

    bool anyLoaded = false;
    for (const IconSource& source : sources) {
        const std::string path = std::string("res/editor/icons/") + source.file + ".svg";
        auto pixels = RasterizeSVG(path.c_str(), source.size);
        if (pixels.empty()) {
            DD_WARN("Could not load editor icon: {}", path);
            continue;
        }
        TextureProperties props(source.size, source.size, TextureFormat::FORMAT_RGBA);
        props.m_MipmapMode = MipmapMode::Generated;
        m_Textures[(size_t)source.icon] = api.CreateTexture(pixels.data(), props);
        anyLoaded = true;
    }
    if (!anyLoaded) return;

    api.SubmitTextureBatch();
    while (!api.PollTextureBatch())
        std::this_thread::yield();

    for (size_t i = 0; i < m_Textures.size(); i++) {
        if (m_Textures[i]) m_ImGuiIDs[i] = api.GetTextureImGuiID(m_Textures[i]);
    }
}
