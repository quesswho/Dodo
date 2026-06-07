#include "EditorIcons.h"

// Implementation is compiled by the nanosvg/nanosvgrast CMake targets.
// Include headers here without the implementation defines.
#include "nanosvg.h"
#include "nanosvgrast.h"

#include <algorithm>
#include <thread>

static constexpr int ICON_SIZE = 20;

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
    return pixels;
}

void EditorIcons::Load(Dodo::RenderAPI& api)
{
    using namespace Dodo;
    const int sz = ICON_SIZE;
    TextureProperties props(sz, sz, TextureFormat::FORMAT_RGBA);

    auto loadIcon = [&](const char* path) -> Ref<Texture> {
        auto px = RasterizeSVG(path, sz);
        if (px.empty())
            return nullptr;
        return api.CreateTexture(px.data(), props);
    };

    m_EntityTex     = loadIcon("res/editor/icon/entity.svg");
    m_EntitySelTex  = loadIcon("res/editor/icon/entity_selected.svg");
    m_EntityRootTex = loadIcon("res/editor/icon/entity_root.svg");

    if (!m_EntityTex && !m_EntitySelTex && !m_EntityRootTex)
        return;

    api.SubmitTextureBatch();
    while (!api.PollTextureBatch())
        std::this_thread::yield();

    if (m_EntityTex)     m_Icons.entity     = api.GetTextureImGuiID(m_EntityTex);
    if (m_EntitySelTex)  m_Icons.entitySel  = api.GetTextureImGuiID(m_EntitySelTex);
    if (m_EntityRootTex) m_Icons.entityRoot  = api.GetTextureImGuiID(m_EntityRootTex);
    m_Icons.ready = true;
}
