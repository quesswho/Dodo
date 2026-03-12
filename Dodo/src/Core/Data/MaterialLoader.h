#pragma once

#include "Core/Graphics/Material/Material.h"
#include "Core/Graphics/Shader/ShaderGenerator.h"

#include <filesystem>

struct aiMaterial;
// enum aiTextureType : int; // Forward declarations of enums are weird. We cast int to aiTextureType in the .cpp.

namespace Dodo {
    class AssetManager;

    class MaterialLoader {
      public:
        Ref<Material> LoadMaterial(const std::string& texture, AssetManager& assets);
        Ref<Material> LoadMaterial(const std::string& path, aiMaterial* material, AssetManager& assets);

      private:
        Ref<Texture> LoadTextureFromMaterial(aiMaterial* material, int type, ShaderBuilderFlags& outFlags,
                                             const std::filesystem::path& modelDir, uint slot);
    };
} // namespace Dodo