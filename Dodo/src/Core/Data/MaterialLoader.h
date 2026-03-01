#pragma once

#include "Core/Graphics/Material/Material.h"
#include "Core/Graphics/Shader/ShaderBuilder.h"

#include <assimp/material.h>
#include <filesystem>

namespace Dodo {

    class MaterialLoader {
      public:
        Ref<Material> LoadMaterial(const char *texture);
        Ref<Material> LoadMaterial(const std::string &path, aiMaterial *material);

      private:
        Ref<Texture> LoadTextureFromMaterial(aiMaterial *material, aiTextureType type, ShaderBuilderFlags &outFlags,
                                             const std::filesystem::path &modelDir, uint slot);
    };
} // namespace Dodo