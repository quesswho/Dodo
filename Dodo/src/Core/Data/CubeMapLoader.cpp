#include "CubeMapLoader.h"
#include "Core/Utilities/Logger.h"

#include <stb_image.h>

namespace Dodo {

    CubeMapData CubeMapLoader::Load(const std::vector<std::string>& paths)
    {
        CubeMapData result;

        if (paths.size() != 6) {
            DD_ERR("CubeMapLoader: requires exactly 6 face paths, got {}", paths.size());
            return result;
        }

        int expectedWidth = 0, expectedHeight = 0;

        for (int i = 0; i < 6; i++) {
            int width, height, channels;
            // Force RGBA so both backends get a consistent format without conversion
            stbi_uc* data = stbi_load(paths[i].c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!data) {
                DD_ERR("CubeMapLoader: failed to load face {}: {}", i, paths[i]);
                return CubeMapData{};
            }

            if (i == 0) {
                expectedWidth = width;
                expectedHeight = height;
            } else if (width != expectedWidth || height != expectedHeight) {
                DD_ERR("CubeMapLoader: face {} has dimensions {}x{}, expected {}x{} (face 0: {})", i, width, height,
                       expectedWidth, expectedHeight, paths[0]);
                stbi_image_free(data);
                return CubeMapData{};
            }

            TextureData& face = result.faces[i];
            face.props.m_Width = (uint)width;
            face.props.m_Height = (uint)height;
            face.props.m_Format = TextureFormat::FORMAT_RGBA;

            const size_t byteCount = (size_t)width * height * 4;
            face.pixels.assign(data, data + byteCount);
            stbi_image_free(data);
        }

        DD_INFO("CubeMapLoader: finished loading cubemap to CPU ({}x{})", expectedWidth, expectedHeight);
        return result;
    }

} // namespace Dodo
