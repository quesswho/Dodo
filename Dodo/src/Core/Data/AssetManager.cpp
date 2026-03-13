#include "AssetManager.h"
#include "pch.h"

#include "Core/Application/Application.h"
#include "Core/Graphics/Shader/ShaderCompiler.h"
#include "Core/Graphics/Shader/ShaderParser.h"
#include "Core/System/FileUtils.h"

namespace Dodo {

    AssetManager::AssetManager() : m_SlangCompiler(SlangCompiler::Target::GLSL)
    {
        Ref<Shader> fallback =
            std::make_shared<Shader>(ShaderCompiler::Compile(ShaderGenerator::GetFallbackShader().source));
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
        Ref<Shader> shader = std::make_shared<Shader>(ShaderCompiler::Compile(source.source));

        int id = m_NextShaderID++;

        shader->Bind();
        int i = 0;
        if (flags & ShaderBuilderFlags::ShaderBuilderFlagCubeMap) shader->SetUniformValue("u_CubeMap", i++);
        if (flags & ShaderBuilderFlags::ShaderBuilderFlagDiffuseMap) shader->SetUniformValue("u_DiffuseMap", i++);
        if (flags & ShaderBuilderFlags::ShaderBuilderFlagSpecularMap) shader->SetUniformValue("u_SpecularMap", i++);
        if (flags & ShaderBuilderFlags::ShaderBuilderFlagNormalMap) shader->SetUniformValue("u_NormalMap", i++);
        if (flags & ShaderBuilderFlags::ShaderBuilderFlagShadowMap) shader->SetUniformValue("u_DepthMap", 3);

        m_ShaderBuilderShaders.emplace(flags, id);
        m_Shaders.emplace(id, std::move(shader));
        return id;
    }

    ShaderID AssetManager::LoadGLSLShaderFromPath(const std::string& path)
    {
        if (m_ShaderPathLookup.find(path) != m_ShaderPathLookup.end()) {
            DD_WARN("Shader already loaded!", path, m_ShaderPathLookup.at(path));
            return m_ShaderPathLookup.at(path);
        }

        ShaderSource source = ShaderParser::Parse(FileUtils::ReadTextFile(path.c_str()));

        Ref<Shader> shader = std::make_shared<Shader>(ShaderCompiler::Compile(source));

        ShaderID id = m_NextShaderID++;

        m_ShaderPathLookup.emplace(path, id);
        m_Shaders.emplace(id, shader);
        return id;
    }

    ShaderID AssetManager::LoadSlangShaderFromPath(const std::string& path)
    {
        if (m_ShaderPathLookup.find(path) != m_ShaderPathLookup.end()) {
            DD_WARN("Shader already loaded!", path, m_ShaderPathLookup.at(path));
            return m_ShaderPathLookup.at(path);
        }

        ShaderSource source = m_SlangCompiler.CompileFile(path);

        Ref<Shader> shader = std::make_shared<Shader>(ShaderCompiler::Compile(source));

        ShaderID id = m_NextShaderID++;

        m_ShaderPathLookup.emplace(path, id);
        m_Shaders.emplace(id, shader);
        return id;
    }

    ShaderID AssetManager::LoadShader(ShaderSource source)
    {
        Ref<Shader> shader = std::make_shared<Shader>(ShaderCompiler::Compile(source));

        int id = m_NextShaderID++;

        m_Shaders.emplace(id, shader);
        return id;
    }

    Ref<Shader> AssetManager::GetShader(ShaderID id)
    {
        auto it = m_Shaders.find(id);
        if (it != m_Shaders.end()) return it->second;
        DD_ERR("Trying to get shader that doesn't exist! ID: {0}", id);
        return nullptr;
    }

    ModelID AssetManager::LoadModel(const std::string& path)
    {
        auto it = m_ModelID.find(path);
        if (it != m_ModelID.end()) {
            DD_WARN("Trying to create model that already exists! {0} ID: {1}", path, it->second);
            return it->second;
        }

        Model* model = m_ModelLoader.LoadModel(path, m_MaterialLoader, *this);
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

        Ref<Material> mat = m_MaterialLoader.LoadMaterial(path, *this);
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