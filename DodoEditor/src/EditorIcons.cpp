#include "EditorIcons.h"

// Implementation is compiled by the nanosvg/nanosvgrast CMake targets.
// Include headers here without the implementation defines.
#include "nanosvg.h"
#include "nanosvgrast.h"

#include <algorithm>
#include <thread>

static constexpr int ENTITY_ICON_SIZE = 20;
static constexpr int ASSET_ICON_SIZE  = 64;

std::vector<unsigned char> EditorIcons::RasterizeSVG(const char* path, int size)
{
    NSVGimage* img = nsvgParseFromFile(path, "px", 96.0f);
    if (!img)
        return {};

    NSVGrasterizer* rast = nsvgCreateRasterizer();
    if (!rast) {
        nsvgDelete(img);
        return {};
    }

    std::vector<unsigned char> pixels(size * size * 4, 0);
    float maxDim = std::max(img->width, img->height);
    float scale  = (maxDim > 0.0f) ? ((float)size / maxDim) : 1.0f;
    nsvgRasterize(rast, img, 0.0f, 0.0f, scale, pixels.data(), size, size, size * 4);

    nsvgDeleteRasterizer(rast);
    nsvgDelete(img);

    // Invert RGB so black-on-transparent icons become white-on-transparent for the dark theme.
    for (size_t i = 0; i < pixels.size(); i += 4) {
        if (pixels[i + 3] > 0) {
            pixels[i]     = 255 - pixels[i];
            pixels[i + 1] = 255 - pixels[i + 1];
            pixels[i + 2] = 255 - pixels[i + 2];
        }
    }

    return pixels;
}

void EditorIcons::Load(Dodo::RenderAPI& api)
{
    using namespace Dodo;

    auto loadIcon = [&](const char* path, int sz) -> Ref<Texture> {
        auto px = RasterizeSVG(path, sz);
        if (px.empty())
            return nullptr;
        TextureProperties props(sz, sz, TextureFormat::FORMAT_RGBA);
        props.m_MipmapMode = MipmapMode::None;
        return api.CreateTexture(px.data(), props);
    };

    m_EntityTex     = loadIcon("res/editor/icon/entity.svg",         ENTITY_ICON_SIZE);
    m_EntitySelTex  = loadIcon("res/editor/icon/entity_selected.svg", ENTITY_ICON_SIZE);
    m_EntityRootTex = loadIcon("res/editor/icon/entity_root.svg",     ENTITY_ICON_SIZE);

    m_FolderTex  = loadIcon("res/editor/icon/folder.svg",  ASSET_ICON_SIZE);
    m_TextureTex = loadIcon("res/editor/icon/texture.svg", ASSET_ICON_SIZE);
    m_ModelTex   = loadIcon("res/editor/icon/model.svg",   ASSET_ICON_SIZE);
    m_ShaderTex  = loadIcon("res/editor/icon/shader.svg",  ASSET_ICON_SIZE);
    m_SceneTex   = loadIcon("res/editor/icon/scene.svg",   ASSET_ICON_SIZE);
    m_FileTex    = loadIcon("res/editor/icon/file.svg",    ASSET_ICON_SIZE);

    bool anyLoaded = m_EntityTex || m_EntitySelTex || m_EntityRootTex
                  || m_FolderTex || m_TextureTex   || m_ModelTex
                  || m_ShaderTex || m_SceneTex     || m_FileTex;
    if (!anyLoaded)
        return;

    api.SubmitTextureBatch();
    while (!api.PollTextureBatch())
        std::this_thread::yield();

    if (m_EntityTex)     m_Icons.entity     = api.GetTextureImGuiID(m_EntityTex);
    if (m_EntitySelTex)  m_Icons.entitySel  = api.GetTextureImGuiID(m_EntitySelTex);
    if (m_EntityRootTex) m_Icons.entityRoot = api.GetTextureImGuiID(m_EntityRootTex);
    if (m_FolderTex)     m_Icons.folder     = api.GetTextureImGuiID(m_FolderTex);
    if (m_TextureTex)    m_Icons.texture    = api.GetTextureImGuiID(m_TextureTex);
    if (m_ModelTex)      m_Icons.model      = api.GetTextureImGuiID(m_ModelTex);
    if (m_ShaderTex)     m_Icons.shader     = api.GetTextureImGuiID(m_ShaderTex);
    if (m_SceneTex)      m_Icons.scene      = api.GetTextureImGuiID(m_SceneTex);
    if (m_FileTex)       m_Icons.file       = api.GetTextureImGuiID(m_FileTex);
    m_Icons.ready = true;
}
