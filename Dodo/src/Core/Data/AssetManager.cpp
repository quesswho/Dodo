#include "AssetManager.h"
#include "pch.h"

#include "Core/Application/Application.h"

namespace Dodo {

    AssetManager::AssetManager() : m_SlangCompiler()
    {
        ShaderAsset fallbackAsset = m_SlangCompiler.CompileFile("res/shader/builtin/Common/Fallback.slang");
        m_Shaders.emplace(0, fallbackAsset);
        m_Pipelines.emplace(0, Application::s_Application->m_RenderAPI->CreatePipeline(PipelineDesc{0}, *this));
    }

    AssetManager::~AssetManager()
    {
        for (auto& model : m_Models)
            delete model.second;
    }

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

    MaterialID AssetManager::LoadMaterial(const std::string& path)
    {
        auto it = m_MaterialID.find(path);
        if (it != m_MaterialID.end()) {
            DD_WARN("Material already loaded: {0}", path);
            return it->second;
        }

        Ref<Material> mat = m_MaterialLoader.LoadMaterial(path, *this, *Application::s_Application->m_RenderAPI);
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

        Model* model = m_ModelLoader.LoadModel(path, m_MaterialLoader, *this, *Application::s_Application->m_RenderAPI);
        if (model == nullptr) {
            DD_ERR("Failed to load model: {0}, Loading default cube", path);
            return GetBuiltinModel(BuiltinModel::Cube);
        }

        int id = m_NextModelID++;

        m_ModelID.emplace(path, id);
        m_ModelPath.emplace(id, path);
        m_Models.emplace(id, model);
        return id;
    }

    ModelID AssetManager::GetBuiltinModel(BuiltinModel type)
    {
        auto it = builtinIDs.find(type);
        if (it != builtinIDs.end()) return it->second;

        Model* model = nullptr;

        switch (type) {
        case BuiltinModel::Cube: {
            std::vector<Mesh*> meshes;
            meshes.push_back(m_MeshFactory.CreateCube(std::make_shared<Material>(Material())));
            model = new Model(meshes);
            break;
        }
        case BuiltinModel::Terrain: {
            std::vector<Mesh*> terrainMeshes;
            terrainMeshes.push_back(
                m_MeshFactory.CreateTerrain(TerrainConfig(), std::make_shared<Material>(Material())));
            model = new Model(terrainMeshes);
            break;
        }
        }

        ModelID id = m_NextModelID++;
        m_Models.emplace(id, model);

        builtinIDs.emplace(type, id);
        return id;
    }

    Model* AssetManager::GetModel(ModelID id)
    {
        auto it = m_Models.find(id);
        if (it != m_Models.end()) return it->second;
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

    ShaderAsset AssetManager::SlangSourceToAsset(const SlangSource& source)
    {
        return m_SlangCompiler.CompileFromString(source.source, source.name);
    }
} // namespace Dodo
