#pragma once

#include "Core/Graphics/Material/Material.h"
#include "Core/Graphics/Material/MaterialFeatures.h"
#include "Core/Graphics/Material/TextureSampler.h"

#include <filesystem>

struct aiMaterial;

namespace Dodo {

    class MaterialLoader {
      public:
        struct MaterialData {
            struct TextureEntry {
                uint slot;
                std::string path;
            };
            std::vector<TextureEntry> textures;
            SamplerProperties samplerProps;
            bool valid = false;
        };

        MaterialData LoadMaterialData(const std::string& path);
        MaterialData LoadMaterialData(const std::string& modelDir, aiMaterial* material);

      private:
        MaterialData LoadMaterialDataFromMtlx(const std::string& mtlxPath);
        std::string GetTexturePath(aiMaterial* material, int type, MaterialFeatures& features,
                                   const std::filesystem::path& modelDir);
    };
} // namespace Dodo
