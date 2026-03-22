#include "AssetManager.h"
#include "pch.h"

#include "Core/Application/Application.h"
#include "Core/Graphics/Pipeline/ShaderCompiler.h"
#include "Core/Graphics/Pipeline/ShaderParser.h"
#include "Core/System/FileUtils.h"

namespace Dodo {

    AssetManager::AssetManager() : m_SlangCompiler(SlangCompiler::Target::GLSL)
    {
        ShaderSource fallback = ShaderGenerator::GetFallbackShader().source;
        m_Shaders.emplace(0, std::move(fallback));
        m_Pipelines.emplace(0,
                            Application::s_Application->m_RenderAPI->CreatePipeline(PipelineDesc{0}, m_Shaders.at(0)));
    }

    AssetManager::~AssetManager()
    {
        for (auto& model : m_Models)
            delete model.second;
    }

    ShaderID AssetManager::LoadShader(ShaderBuilderFlags flags, RenderAPI& renderAPI)
    {
        if (m_ShaderBuilderShaders.count(flags)) return m_ShaderBuilderShaders[flags];

        GeneratedShaderSource source = ShaderGenerator::Generate(flags);
        ShaderID id = m_NextShaderID++;
        m_ShaderBuilderShaders.emplace(flags, id);
        m_Shaders.emplace(id, std::move(source.source));
        return id;
    }

    ShaderID AssetManager::LoadShaderFromPath(const std::string& path)
    {
        if (m_ShaderPathLookup.count(path)) {
            DD_WARN("Shader already loaded! {}", path);
            return m_ShaderPathLookup.at(path);
        }

        ShaderSource source = path.ends_with(".slang") ? m_SlangCompiler.CompileFile(path)
                                                       : ShaderParser::Parse(FileUtils::ReadTextFile(path.c_str()));

        ShaderID id = m_NextShaderID++;
        m_ShaderPathLookup.emplace(path, id);
        m_Shaders.emplace(id, std::move(source));

        return id;
    }

    ShaderID AssetManager::LoadShader(ShaderSource source)
    {
        ShaderID id = m_NextShaderID++;
        m_Shaders.emplace(id, std::move(source));
        return id;
    }

    PipelineID AssetManager::CreatePipeline(const PipelineDesc& desc, RenderAPI& renderAPI)
    {
        auto shaderIt = m_Shaders.find(desc.shaderID);
        if (shaderIt == m_Shaders.end()) {
            DD_ERR("CreatePipeline: shader not found!");
            return 0;
        }

        Ref<Pipeline> pipeline = renderAPI.CreatePipeline(desc, shaderIt->second);
        PipelineID id = m_NextPipelineID++;
        m_Pipelines.emplace(id, pipeline);
        return id;
    }

    PipelineID AssetManager::CreatePipeline(ShaderBuilderFlags flags, RenderAPI& renderAPI)
    {
        // Return existing pipeline if already created for these flags
        if (m_ShaderBuilderPipelines.count(flags)) {
            return m_ShaderBuilderPipelines.at(flags);
        }

        ShaderID shaderID = LoadShader(flags, renderAPI);
        ShaderSource source = m_Shaders.at(shaderID);

        PipelineDesc desc;
        desc.shaderID = shaderID;

        Ref<Pipeline> pipeline = renderAPI.CreatePipeline(desc, source);
        PipelineID pipelineID = m_NextPipelineID++;
        m_ShaderBuilderPipelines.emplace(flags, pipelineID);
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
            // TODO: We should ideally have a fallback material stored in asset manager instead of creating a new one
            meshes.push_back(m_MeshFactory.CreateCube(std::make_shared<Material>(Material())));
            model = new Model(meshes);
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
} // namespace Dodo