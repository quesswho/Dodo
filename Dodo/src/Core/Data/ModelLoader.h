#pragma once

#include "Core/Graphics/Material/MaterialFeatures.h"
#include "Core/Graphics/Scene/Model.h"
#include "TextureLoader.h"

struct aiMesh;
struct aiMaterial;

namespace Dodo {
    class AssetManager;
    class MaterialLoader;

    struct ModelLoader {

        struct Vertex {
            Math::Vec3 m_Position;
            Math::Vec2 m_Texcoord;
            Math::Vec3 m_Normal;
            Math::Vec3 m_Tangent;
        };

        struct ModelData {
            struct TextureEntry {
                int slot;
                std::string path;
                TextureData pixels;
            };
            struct MaterialEntry {
                std::vector<TextureEntry> textures;
                MaterialFeatures features = MaterialFeatures::None;
            };
            struct MeshEntry {
                std::vector<Vertex> vertices;
                std::vector<uint> indices;
                uint materialIndex;
            };
            std::vector<MeshEntry> meshes;
            std::vector<MaterialEntry> materials;
            bool failed = false;
        };

        // Load model data to CPU memory
        ModelData LoadModelData(const std::string& path, TextureLoader& textureLoader);

        // Uploads CPU data to the GPU
        Ref<Model> BuildModel(const ModelData& data, const std::vector<Ref<Material>>& materials);
      private:
        Ref<Mesh> LoadMesh(::aiMesh* mesh, Ref<Material> material);
    };
} // namespace Dodo
