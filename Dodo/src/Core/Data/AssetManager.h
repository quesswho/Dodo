#pragma once

#include <Core/Common.h>
#include <mutex>

#include "AssetTypes.h"
#include "Core/Graphics/CubeMap.h"
#include "Core/Graphics/Material/Texture.h"
#include "Core/Graphics/Pipeline/Pipeline.h"
#include "Core/Graphics/Pipeline/PipelineDesc.h"
#include "Core/Graphics/Pipeline/ShaderSource.h"
#include "Core/Graphics/Pipeline/SlangCompiler.h"
#include "Core/Graphics/Scene/Mesh/MeshFactory.h"
#include "Core/Graphics/Scene/Model.h"
#include "CubeMapLoader.h"
#include "MaterialLoader.h"
#include "ModelLoader.h"
#include "ShaderAsset.h"
#include "TextureLoader.h"

namespace Dodo {

    class ThreadManager;

    enum class BuiltinModel {
        Cube,
        Terrain,
    };

    class AssetManager {
      public:
        AssetManager(RenderAPI& renderAPI, ThreadManager& threadManager);
        ~AssetManager();

        ShaderID LoadShaderFromPath(const std::string& path);
        ShaderID LoadShader(SlangSource source);
        void ReloadShader(const std::string& path, RenderAPI& renderAPI);
        ShaderAsset& GetShaderAsset(ShaderID id);
        const ShaderAsset& GetShaderAsset(ShaderID id) const;

        PipelineID CreatePipeline(const PipelineDesc& desc, RenderAPI& renderAPI);
        PipelineID CreatePipeline(MaterialFeatures features, RenderAPI& renderAPI);
        Ref<Pipeline> GetPipeline(PipelineID id);
        Ref<Pipeline> GetFallbackPipeline() const { return m_Pipelines.at(0); }

        TextureID LoadTexture(const std::string& path);
        Ref<Texture> GetTexture(TextureID id);
        AssetState GetTextureState(TextureID id) const;

        CubeMapID LoadCubeMap(const std::vector<std::string>& paths);
        CubeMapID CreateCubeMapFromEquirectangular(const std::string& hdrPath, uint faceSize);
        CubeMapID CreateIrradianceMap(CubeMapID envMapID, uint faceSize);
        Ref<CubeMap> GetCubeMap(CubeMapID id); // Returns nullptr if still loading

        MaterialID LoadMaterial(const std::string& path);
        Ref<Material> GetMaterial(MaterialID id);
        AssetState GetMaterialState(MaterialID id) const;

        /**
         * Loads a model from a file, or returns the builtin model when the path is a builtin name
         * such as "builtin:cube".
         */
        ModelID LoadModel(const std::string& path);
        ModelID GetBuiltinModel(BuiltinModel type);
        Ref<Model> GetModel(ModelID id);
        AssetState GetModelState(ModelID id) const;

        Ref<VertexBuffer> GetScreenQuadBuffer(RenderAPI& renderAPI);

        // Uploads all staged CPU data to the GPU. Must be called from the render thread.
        void FlushStagingQueue(RenderAPI& renderAPI);

        // Finalizes any GPU uploads whose fence has signaled and registers the textures/cubemaps.
        // Called at the start of FlushStagingQueue; also safe to call at other frame boundaries.
        void FinalizeReadyUploads(RenderAPI& renderAPI);

        /**
         * Returns the string that loads the model again through LoadModel: the file path, or the builtin
         * name for builtin models.
         */
        std::string GetModelPath(ModelID id);

        /**
         * @return True if the model was loaded from a file, false for builtin models.
         */
        bool HasPath(ModelID id) const { return m_ModelPath.find(id) != m_ModelPath.end(); }

      private:
        ShaderAsset SlangSourceToAsset(const SlangSource& source);

        /**
         * Returns the material builtin models are drawn with: lit, with a plain light grey albedo.
         * Created on first use and shared between all builtin models.
         */
        Ref<Material> GetDefaultMaterial();

        /**
         * Creates a 1x1 texture of the given color, for materials that have a color instead of a texture.
         *
         * @param color RGBA in the 0 to 1 range, clamped.
         */
        Ref<Texture> CreateSolidColorTexture(const Math::Vec4& color);

        // Names builtin models are referred to by wherever a model path is stored, such as scene files
        struct BuiltinModelName {
            BuiltinModel type;
            const char* name;
        };
        static constexpr BuiltinModelName s_BuiltinModelNames[] = {
            {BuiltinModel::Cube, "builtin:cube"},
            {BuiltinModel::Terrain, "builtin:terrain"},
        };

        RenderAPI& m_RenderAPI;
        ThreadManager& m_ThreadManager;

        ModelLoader m_ModelLoader;
        MaterialLoader m_MaterialLoader;
        MeshFactory m_MeshFactory;
        SlangCompiler m_SlangCompiler;
        TextureLoader m_TextureLoader;
        CubeMapLoader m_CubeMapLoader;

        std::unordered_map<ShaderID, ShaderAsset> m_Shaders;
        std::unordered_map<std::string, ShaderID> m_ShaderPathLookup;
        std::unordered_map<MaterialFeatures, ShaderID> m_ShaderBuilderShaders;

        std::unordered_map<PipelineID, Ref<Pipeline>> m_Pipelines;
        std::unordered_map<MaterialFeatures, PipelineID> m_ShaderBuilderPipelines;

        std::unordered_map<TextureID, Ref<Texture>> m_Textures;
        std::unordered_map<std::string, TextureID> m_TexturePathLookup;
        std::unordered_map<TextureID, AssetState> m_TextureStates;

        std::unordered_map<CubeMapID, Ref<CubeMap>> m_CubeMaps;
        std::unordered_map<CubeMapID, AssetState> m_CubeMapStates;

        std::unordered_map<MaterialID, Ref<Material>> m_Materials;
        std::unordered_map<std::string, MaterialID> m_MaterialID;
        std::unordered_map<MaterialID, AssetState> m_MaterialStates;
        Ref<Material> m_DefaultMaterial;

        std::unordered_map<ModelID, Ref<Model>> m_Models;     // Stores id as key and model pointer as value
        std::unordered_map<std::string, ModelID> m_ModelID;   // Stores path as key and id as value
        std::unordered_map<ModelID, std::string> m_ModelPath; // Stores id as key and path as value
        std::unordered_map<BuiltinModel, ModelID> builtinIDs;
        std::unordered_map<ModelID, AssetState> m_ModelStates;

        // GPU-submitted but not yet fence-signaled: textures/cubemaps whose staging buffers are
        // still in use by the GPU. Moved to m_Textures/m_CubeMaps once PollTextureBatch returns true.
        struct PendingGPUTexture {
            TextureID id;
            Ref<Texture> texture;
        };
        struct PendingGPUCubeMap {
            CubeMapID id;
            Ref<CubeMap> cubeMap;
        };
        std::vector<PendingGPUTexture> m_PendingGPUTextures;
        std::vector<PendingGPUCubeMap> m_PendingGPUCubeMaps;

        // Staging queues written by worker threads, drained by main thread in FlushStagingQueue
        struct PendingTextureUpload {
            TextureID id;
            TextureData data;
        };
        struct PendingCubeMapUpload {
            CubeMapID id;
            CubeMapData data;
        };
        // Written by Assimp worker; processed by FlushStagingQueue which dispatches per-texture LoadTexture tasks
        struct PendingModelUpload {
            ModelID id;
            ModelLoader::ModelData modelData;
        };
        // Built by FlushStagingQueue once Assimp is done; held until all texture IDs are in m_Textures
        struct PendingModelAssembly {
            ModelID id;
            ModelLoader::ModelData modelData;
            std::vector<TextureID> waitingFor; // texture IDs that must be in m_Textures before building
        };
        std::mutex m_StagingMutex;
        std::vector<PendingTextureUpload> m_StagingTextures;
        std::vector<TextureID> m_FailedTextureIDs;
        std::vector<PendingCubeMapUpload> m_StagingCubeMaps;
        std::vector<CubeMapID> m_FailedCubeMapIDs;
        std::vector<PendingModelUpload> m_StagingModels;
        std::vector<ModelID> m_FailedModels;
        std::vector<PendingModelAssembly> m_PendingModelAssemblies;

        struct PendingMaterialAssembly {
            MaterialID id;
            Ref<Material> material;
            std::vector<std::pair<uint, TextureID>> waitingFor;
        };
        std::vector<PendingMaterialAssembly> m_PendingMaterialAssemblies;

        ShaderID m_NextShaderID = 1;
        PipelineID m_NextPipelineID = 1;
        MaterialID m_NextMaterialID = 1;
        ModelID m_NextModelID = 1;
        TextureID m_NextTextureID = 1;
        CubeMapID m_NextCubeMapID = 1;
    };
} // namespace Dodo
