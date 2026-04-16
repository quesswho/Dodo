#pragma once

#include "Core/Graphics/Material/MaterialFeatures.h"
#include "Core/Graphics/Pipeline/PipelineDesc.h"
#include "Core/Graphics/RenderAPI.h"
#include "Core/Graphics/Scene/Model.h"

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
            Math::Vec4 m_Tangent; // xyz = tangent direction, w = bitangent sign (+1 or -1)
        };

        struct ModelData {
            struct TextureEntry {
                int slot;
                std::string path;
            };
            struct MaterialEntry {
                std::vector<TextureEntry> textures;
                MaterialFeatures features = MaterialFeatures::None;
                BlendMode blendMode = BlendMode::Opaque;
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

        // Load model and texture paths to CPU memory. Thread-safe.
        ModelData LoadModelData(const std::string& path);

        // Uploads CPU mesh data to the GPU
        Ref<Model> BuildModel(const ModelData& data, const std::vector<Ref<Material>>& materials, RenderAPI& renderAPI);
    };
} // namespace Dodo
