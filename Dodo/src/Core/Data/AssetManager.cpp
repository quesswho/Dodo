#include "AssetManager.h"
#include "pch.h"

#include "Core/System/ThreadManager.h"

namespace Dodo {

    AssetManager::AssetManager(RenderAPI& renderAPI, ThreadManager& threadManager)
        : m_RenderAPI(renderAPI), m_ThreadManager(threadManager), m_SlangCompiler()
    {
        ShaderAsset fallbackAsset = m_SlangCompiler.CompileFile("res/shader/builtin/Common/Fallback.slang");
        m_Shaders.emplace(0, fallbackAsset);
        PipelineDesc fallbackDesc;
        fallbackDesc.vertexLayout = BufferProperties({{"POSITION", 3}, {"TEXCOORD", 2}, {"NORMAL", 3}, {"TANGENT", 4}});
        m_Pipelines.emplace(0, m_RenderAPI.CreatePipeline(fallbackDesc, *this));
    }

    AssetManager::~AssetManager() {}

    /**
     * Loads a slang shader from path.
     * Returns 0 on fail
     */
    ShaderID AssetManager::LoadShaderFromPath(const std::string& path)
    {
        if (m_ShaderPathLookup.count(path)) {
            DD_WARN("Shader already loaded! {}", path);
            return m_ShaderPathLookup.at(path);
        }

        ShaderAsset source = m_SlangCompiler.CompileFile(path);
        if (source.stages.empty()) {
            DD_ERR("Failed to compile slang shader: {}", path);
            return 0; // Intentionally returns id of fallback shader
        }

        ShaderID id = m_NextShaderID++;
        m_ShaderPathLookup.emplace(path, id);
        m_Shaders.emplace(id, std::move(source));

        return id;
    }

    ShaderID AssetManager::LoadShader(SlangSource source)
    {
        ShaderAsset asset = SlangSourceToAsset(source);
        if (asset.stages.empty()) {
            DD_ERR("Failed to compile slang shader source: {}", source.name);
            return 0;
        }

        ShaderID id = m_NextShaderID++;
        m_Shaders.emplace(id, std::move(asset));
        return id;
    }

    ShaderAsset& AssetManager::GetShaderAsset(ShaderID id)
    {
        auto it = m_Shaders.find(id);
        if (it != m_Shaders.end()) return it->second;
        DD_ERR("Trying to get shader asset that doesn't exist! Returning fallback shader instead. ID: {0}", id);
        return m_Shaders.at(0);
    }

    const ShaderAsset& AssetManager::GetShaderAsset(ShaderID id) const
    {
        auto it = m_Shaders.find(id);
        if (it != m_Shaders.end()) return it->second;
        DD_ERR("Trying to get shader asset that doesn't exist! Returning fallback shader instead. ID: {0}", id);
        return m_Shaders.at(0);
    }

    PipelineID AssetManager::CreatePipeline(const PipelineDesc& desc, RenderAPI& renderAPI)
    {
        auto shaderIt = m_Shaders.find(desc.shaderID);
        if (shaderIt == m_Shaders.end()) {
            DD_ERR("CreatePipeline: shader not found!");
            return 0;
        }

        if (shaderIt->second.stages.empty()) {
            DD_ERR("CreatePipeline: shader has no pipeline-compatible stages!");
            return 0;
        }

        Ref<Pipeline> pipeline = renderAPI.CreatePipeline(desc, *this);
        PipelineID id = m_NextPipelineID++;
        m_Pipelines.emplace(id, pipeline);
        return id;
    }

    PipelineID AssetManager::CreatePipeline(MaterialFeatures features, RenderAPI& renderAPI)
    {
        if (m_ShaderBuilderPipelines.count(features)) {
            return m_ShaderBuilderPipelines.at(features);
        }

        ShaderID shaderID = LoadShaderFromPath("res/shader/builtin/Passes/ForwardLit.slang");
        PipelineDesc desc;
        desc.shaderID = shaderID;

        Ref<Pipeline> pipeline = renderAPI.CreatePipeline(desc, *this);
        PipelineID pipelineID = m_NextPipelineID++;
        m_ShaderBuilderPipelines.emplace(features, pipelineID);
        m_Pipelines.emplace(pipelineID, pipeline);
        return pipelineID;
    }

    Ref<Pipeline> AssetManager::GetPipeline(PipelineID id)
    {
        auto it = m_Pipelines.find(id);
        if (it != m_Pipelines.end()) return it->second;
        DD_ERR("Trying to get pipeline that doesn't exist! ID: {0}", id);
        return nullptr;
    }

    TextureID AssetManager::LoadTexture(const std::string& path)
    {
        auto it = m_TexturePathLookup.find(path);
        if (it != m_TexturePathLookup.end()) return it->second;

        TextureID id = m_NextTextureID++;
        m_TexturePathLookup.emplace(path, id);
        m_TextureStates.emplace(id, AssetState::Loading);

        // Setup async task to load texture data
        m_ThreadManager.Task([this, id, path]() {
            TextureData data = m_TextureLoader.Load(path);
            std::lock_guard<std::mutex> lock(m_StagingMutex);
            if (data.pixels.empty())
                m_FailedTextureIDs.push_back(id);
            else
                m_StagingTextures.push_back({id, std::move(data)});
        });

        return id;
    }

    Ref<Texture> AssetManager::GetTexture(TextureID id)
    {
        auto it = m_Textures.find(id);
        if (it != m_Textures.end()) return it->second;
        DD_ERR("Trying to get texture that doesn't exist! ID: {0}", id);
        return nullptr;
    }

    CubeMapID AssetManager::LoadCubeMap(const std::vector<std::string>& paths)
    {
        CubeMapID id = m_NextCubeMapID++;
        m_CubeMapStates.emplace(id, AssetState::Loading);

        m_ThreadManager.Task([this, id, paths]() {
            CubeMapData data = m_CubeMapLoader.Load(paths);
            std::lock_guard<std::mutex> lock(m_StagingMutex);
            if (data.faces[0].pixels.empty())
                m_FailedCubeMapIDs.push_back(id);
            else
                m_StagingCubeMaps.push_back({id, std::move(data)});
        });

        return id;
    }

    Ref<CubeMap> AssetManager::GetCubeMap(CubeMapID id)
    {
        auto it = m_CubeMaps.find(id);
        if (it != m_CubeMaps.end()) return it->second;
        return nullptr;
    }

    MaterialID AssetManager::LoadMaterial(const std::string& path)
    {
        auto it = m_MaterialID.find(path);
        if (it != m_MaterialID.end()) {
            DD_WARN("Material already loaded: {0}", path);
            return it->second;
        }

        Ref<Material> mat = m_MaterialLoader.LoadMaterial(path, *this, m_RenderAPI);
        MaterialID id = m_NextMaterialID++;

        m_MaterialID.emplace(path, id);
        m_Materials.emplace(id, std::move(mat));
        return id;
    }

    Ref<Material> AssetManager::GetMaterial(MaterialID id)
    {
        auto it = m_Materials.find(id);
        if (it != m_Materials.end()) return it->second;
        DD_ERR("Trying to get material that doesn't exist! ID: {0}", id);
        return nullptr;
    }

    ModelID AssetManager::LoadModel(const std::string& path)
    {
        auto it = m_ModelID.find(path);
        if (it != m_ModelID.end()) {
            DD_WARN("Trying to create model that already exists! {0} ID: {1}", path, it->second);
            return it->second;
        }

        ModelID id = m_NextModelID++;
        m_ModelID.emplace(path, id);
        m_ModelPath.emplace(id, path);
        m_ModelStates.emplace(id, AssetState::Loading);

        m_ThreadManager.Task([this, id, path]() {
            ModelLoader::ModelData modelData = m_ModelLoader.LoadModelData(path);
            std::lock_guard<std::mutex> lock(m_StagingMutex);
            if (modelData.failed)
                m_FailedModels.push_back(id);
            else
                m_StagingModels.push_back({id, std::move(modelData)});
        });

        return id;
    }

    AssetState AssetManager::GetModelState(ModelID id) const
    {
        auto it = m_ModelStates.find(id);
        if (it != m_ModelStates.end()) return it->second;
        return AssetState::NotLoaded;
    }

    void AssetManager::FinalizeReadyUploads(RenderAPI& renderAPI)
    {
        if (m_PendingGPUTextures.empty() && m_PendingGPUCubeMaps.empty()) return;
        if (!renderAPI.PollTextureBatch()) return;

        for (auto& pending : m_PendingGPUTextures) {
            m_Textures.emplace(pending.id, std::move(pending.texture));
            m_TextureStates[pending.id] = AssetState::Loaded;
        }
        m_PendingGPUTextures.clear();

        for (auto& pending : m_PendingGPUCubeMaps) {
            m_CubeMaps.emplace(pending.id, std::move(pending.cubeMap));
            m_CubeMapStates[pending.id] = AssetState::Loaded;
        }
        m_PendingGPUCubeMaps.clear();
    }

    void AssetManager::FlushStagingQueue(RenderAPI& renderAPI)
    {
        FinalizeReadyUploads(renderAPI);
        std::vector<PendingTextureUpload> textures;
        std::vector<TextureID> failedTextureIDs;
        std::vector<PendingCubeMapUpload> cubeMaps;
        std::vector<CubeMapID> failedCubeMapIDs;
        std::vector<PendingModelUpload> models;
        std::vector<ModelID> failedModels;
        {
            std::lock_guard<std::mutex> lock(m_StagingMutex);
            std::swap(textures, m_StagingTextures);
            std::swap(failedTextureIDs, m_FailedTextureIDs);
            std::swap(cubeMaps, m_StagingCubeMaps);
            std::swap(failedCubeMapIDs, m_FailedCubeMapIDs);
            std::swap(models, m_StagingModels);
            std::swap(failedModels, m_FailedModels);
        }

        for (TextureID id : failedTextureIDs) {
            DD_ERR("Async texture load failed, ID: {}", id);
            m_TextureStates[id] = AssetState::Failed;
        }

        for (auto& pending : textures) {
            Ref<Texture> tex = renderAPI.CreateTexture(pending.data.pixels.data(), pending.data.props);
            m_PendingGPUTextures.push_back({pending.id, std::move(tex)});
            m_TextureStates[pending.id] = AssetState::Staging;
        }

        for (CubeMapID id : failedCubeMapIDs) {
            DD_ERR("Async cubemap load failed, ID: {}", id);
            m_CubeMapStates[id] = AssetState::Failed;
        }

        for (auto& pending : cubeMaps) {
            Ref<CubeMap> cubeMap = renderAPI.CreateCubeMap(pending.data);
            m_PendingGPUCubeMaps.push_back({pending.id, std::move(cubeMap)});
            m_CubeMapStates[pending.id] = AssetState::Staging;
        }

        for (ModelID id : failedModels) {
            DD_ERR("Async model load failed, ID: {}", id);
            m_ModelStates[id] = AssetState::Failed;
        }

        // Assimp parse is done. Dispatch one LoadTexture task per unique texture path,
        // then put the model in m_PendingModelAssemblies until all textures are ready.
        for (auto& pending : models) {
            PendingModelAssembly assembly;
            assembly.id = pending.id;
            assembly.modelData = std::move(pending.modelData);

            for (const auto& matEntry : assembly.modelData.materials) {
                for (const auto& texEntry : matEntry.textures) {
                    TextureID texID = LoadTexture(texEntry.path); // dispatches async if not already loading
                    assembly.waitingFor.push_back(texID);
                }
            }

            m_PendingModelAssemblies.push_back(std::move(assembly));
        }

        // Check which models whose textures have all been uploaded, and build them
        auto it = m_PendingModelAssemblies.begin();
        while (it != m_PendingModelAssemblies.end()) {
            bool ready = true;
            for (TextureID texID : it->waitingFor) {
                if (!m_Textures.count(texID)) {
                    ready = false;
                    break;
                }
            }
            if (!ready) { // If not all textures are ready, skip for now
                ++it;
                continue;
            }

            const auto& modelData = it->modelData;
            std::vector<Ref<Material>> materials;
            materials.reserve(modelData.materials.size());

            // Track which texture in waitingFor corresponds to which material/slot
            size_t texIdx = 0;
            size_t matIdx = 0;
            for (const auto& matEntry : modelData.materials) {
                Ref<Material> material = std::make_shared<Material>();
                for (const auto& texEntry : matEntry.textures) {
                    TextureID texID = it->waitingFor[texIdx++];
                    material->AddTexture(texEntry.slot, m_Textures.at(texID));
                }
                bool hasTextures = !matEntry.textures.empty();
                bool hasColorFallback = !hasTextures && matEntry.albedoColor.has_value();

                if (hasColorFallback) {
                    const Math::Vec4& c = *matEntry.albedoColor;
                    uchar pixels[4] = {
                        static_cast<uchar>(std::clamp(c.x, 0.0f, 1.0f) * 255.0f),
                        static_cast<uchar>(std::clamp(c.y, 0.0f, 1.0f) * 255.0f),
                        static_cast<uchar>(std::clamp(c.z, 0.0f, 1.0f) * 255.0f),
                        static_cast<uchar>(std::clamp(c.w, 0.0f, 1.0f) * 255.0f),
                    };
                    material->AddTexture(0, renderAPI.CreateTexture(pixels, TextureProperties(1, 1, TextureFormat::FORMAT_RGBA)));
                }

                if (hasTextures || hasColorFallback) {
                    ShaderID shaderID = LoadShaderFromPath("res/shader/builtin/Passes/ForwardLit.slang");
                    PipelineDesc desc;
                    desc.shaderID = shaderID;
                    desc.blendMode = matEntry.blendMode;
                    desc.depthWrite = (matEntry.blendMode != BlendMode::AlphaBlend);
                    material->SetShader(GetPipeline(CreatePipeline(desc, renderAPI)));
                    material->SetSampler(renderAPI.CreateSampler(SamplerProperties()));
                } else {
                    DD_WARN("ModelLoader: Material {} (model ID {}) has no textures or color, using fallback pipeline", matIdx, it->id);
                }
                materials.push_back(std::move(material));
                matIdx++;
            }

            m_Models.emplace(it->id, m_ModelLoader.BuildModel(modelData, materials, renderAPI));
            m_ModelStates[it->id] = AssetState::Loaded;
            it = m_PendingModelAssemblies.erase(it);
        }

        renderAPI.SubmitTextureBatch();
    }

    ModelID AssetManager::GetBuiltinModel(BuiltinModel type)
    {
        auto it = builtinIDs.find(type);
        if (it != builtinIDs.end()) return it->second;

        Ref<Model> model = nullptr;

        switch (type) {
        case BuiltinModel::Cube: {
            std::vector<Ref<Mesh>> meshes;
            meshes.push_back(m_MeshFactory.CreateCube(std::make_shared<Material>(Material()), m_RenderAPI));
            model = std::make_shared<Model>(meshes);
            break;
        }
        case BuiltinModel::Terrain: {
            std::vector<Ref<Mesh>> terrainMeshes;
            terrainMeshes.push_back(
                m_MeshFactory.CreateTerrain(TerrainConfig(), std::make_shared<Material>(Material()), m_RenderAPI));
            model = std::make_shared<Model>(terrainMeshes);
            break;
        }
        }

        ModelID id = m_NextModelID++;
        m_Models.emplace(id, model);

        builtinIDs.emplace(type, id);
        return id;
    }

    Ref<Model> AssetManager::GetModel(ModelID id)
    {
        auto it = m_Models.find(id);
        if (it != m_Models.end()) return it->second;

        if (m_ModelStates.count(id))
            return GetModel(GetBuiltinModel(BuiltinModel::Cube)); // Model still loading, return fallback cube model

        DD_ERR("Trying to get model that doesn't exist! ID: {0}", id);
        return nullptr;
    }

    std::string AssetManager::GetModelPath(ModelID id)
    {
        auto it = m_ModelPath.find(id);
        if (it == m_ModelPath.end()) {
            DD_ERR("Trying to get path of model that doesn't exist! ID: {0}", id);
            return "";
        }
        return it->second;
    }

    Ref<VertexBuffer> AssetManager::GetScreenQuadBuffer(RenderAPI& renderAPI)
    {
        return m_MeshFactory.GetScreenQuadBuffer(renderAPI);
    }

    ShaderAsset AssetManager::SlangSourceToAsset(const SlangSource& source)
    {
        return m_SlangCompiler.CompileFromString(source.source, source.name);
    }
} // namespace Dodo
