#include "AssetManager.h"
#include "pch.h"

#include "Core/Application/Application.h"
#include "Core/Graphics/Shader/ShaderCompiler.h"
#include "Core/Graphics/Shader/ShaderParser.h"
#include "Core/System/FileUtils.h"

namespace Dodo {

    AssetManager::AssetManager(bool serialization)
        : m_Serialization(serialization), m_ModelLoader(new ModelLoader), m_MaterialLoader(new MaterialLoader),
          m_MeshFactory(new MeshFactory())
    {
        Ref<Shader> fallback = std::make_unique<Shader>(ShaderCompiler::Compile(ShaderGenerator::GetFallbackShader().source));
        m_Shaders.emplace(0, fallback);
    }

    AssetManager::~AssetManager()
    {
        for (auto& model : m_Models)
            delete model.second;
    }

    ShaderID AssetManager::LoadShader(ShaderBuilderFlags flags)
    {
        if (m_ShaderBuilderShaders.find(flags) != m_ShaderBuilderShaders.end()) return m_ShaderBuilderShaders[flags];

        GeneratedShaderSource source = ShaderGenerator::Generate(flags);
        Ref<Shader> shader = std::make_unique<Shader>(ShaderCompiler::Compile(source.source));

        int id = m_Shaders.size();

        m_ShaderBuilderShaders.emplace(flags, id);
        m_Shaders.emplace(id, shader);

        shader->Bind();
        int i = 0;
        if (flags & ShaderBuilderFlags::ShaderBuilderFlagCubeMap) shader->SetUniformValue("u_CubeMap", i++);
        if (flags & ShaderBuilderFlags::ShaderBuilderFlagDiffuseMap) shader->SetUniformValue("u_DiffuseMap", i++);
        if (flags & ShaderBuilderFlags::ShaderBuilderFlagSpecularMap) shader->SetUniformValue("u_SpecularMap", i++);
        if (flags & ShaderBuilderFlags::ShaderBuilderFlagNormalMap) shader->SetUniformValue("u_NormalMap", i++);
        if (flags & ShaderBuilderFlags::ShaderBuilderFlagShadowMap) shader->SetUniformValue("u_DepthMap", 3);

        return id;
    }

    ShaderID AssetManager::LoadShaderFromPath(const std::string& path) 
    {
        if (m_ShaderPathLookup.find(path) != m_ShaderPathLookup.end()) {
            DD_WARN("Shader already loaded!", path, m_ShaderPathLookup.at(path));
            return m_ShaderPathLookup.at(path);
        }

        ShaderSource source = ShaderParser::Parse(FileUtils::ReadTextFile(path.c_str()));

        Ref<Shader> shader = std::make_shared<Shader>(ShaderCompiler::Compile(source));

        int id = m_Shaders.size();

        m_ShaderPathLookup.emplace(path, id);
        m_Shaders.emplace(id, shader);
        return id;
    }

    ShaderID AssetManager::LoadShader(ShaderSource source) {
        Ref<Shader> shader = std::make_shared<Shader>(ShaderCompiler::Compile(source));

        int id = m_Shaders.size();

        m_Shaders.emplace(id, shader);
        return id;
    }

    Ref<Shader> AssetManager::GetShader(ShaderID id)
    {
        if (m_Shaders.find(id) != m_Shaders.end()) return m_Shaders[id];
        DD_ERR("Trying to get shader that doesn't exist! ID: {0}", id);
        return nullptr;
    }

    ModelID AssetManager::LoadModel(const std::string& path)
    {
        if (m_ModelID.find(path) != m_ModelID.end()) {
            DD_WARN("Trying to create model that already exists! {0} ID: {1}", path, m_ModelID.at(path));
            return m_ModelID.at(path);
        }

        Model* model = m_ModelLoader->LoadModel(path);
        if (model == nullptr) {
            DD_ERR("Failed to load model: {0}, Loading default cube", path);
            return GetBuiltinModel(BuiltinModel::Cube);
        }

        int id = m_Models.size();

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
            meshes.push_back(m_MeshFactory->CreateCube(std::make_shared<Material>(Material())));
            model = new Model(meshes);
            break;
        }
        }

        ModelID id = m_Models.size();
        m_Models.emplace(id, model);

        builtinIDs.emplace(type, id);
        return id;
    }

    Model* AssetManager::GetModel(ModelID id)
    {
        if (m_Models.find(id) != m_Models.end()) return m_Models[id];
        DD_ERR("Trying to get model that doesn't exist! ID: {0}", id);
        return nullptr;
    }

    std::string AssetManager::GetModelPath(ModelID id)
    {
        if (m_ModelPath.find(id) == m_ModelPath.end()) {
            DD_ERR("Trying to get path of model that doesn't exist! ID: {0}", id);
            return "";
        }
        return m_ModelPath[id];
    }

    Ref<Material> AssetManager::GetMaterial(const char* path)
    {
        if (m_MaterialID.find(path) != m_MaterialID.end()) {

            return m_Materials[m_MaterialID.at(path)];
        }

        Ref<Material> mat = m_MaterialLoader->LoadMaterial(path);
        int id = m_Materials.size();
        m_MaterialID.emplace(path, id);
        m_Materials.emplace(id, mat);
        return mat;
    }
} // namespace Dodo