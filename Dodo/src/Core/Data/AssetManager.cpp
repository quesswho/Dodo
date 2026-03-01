#include "AssetManager.h"
#include "pch.h"

#include "Core/Application/Application.h"

namespace Dodo {

    AssetManager::AssetManager(bool serialization)
        : m_Serialization(serialization), m_ModelLoader(new ModelLoader), m_MaterialLoader(new MaterialLoader),
          m_MeshFactory(new MeshFactory())
    {}

    AssetManager::~AssetManager()
    {
        for (auto &model : m_Models)
            delete model.second;
    }

    Ref<Shader> AssetManager::GetShader(ShaderBuilderFlags flags)
    {
        if (m_ShaderBuilderShaders.find(flags) != m_ShaderBuilderShaders.end())
            return m_ShaderBuilderShaders[flags];

        m_ShaderBuilderShaders.insert(std::make_pair(
            flags, Application::s_Application->m_RenderAPI->m_ShaderBuilder->BuildVertexFragmentShader(flags)));
        return m_ShaderBuilderShaders[flags];
    }

    ModelID AssetManager::LoadModel(const std::string &path)
    {
        if (m_ModelID.find(path) != m_ModelID.end())
        {
            DD_WARN("Trying to create model that already exists! {0} ID: {1}", path, m_ModelID.at(path));
            return m_ModelID.at(path);
        }

        Model *model = m_ModelLoader->LoadModel(path);
        int id = m_Models.size();

        m_ModelID.emplace(path, id);
        m_ModelPath.emplace(id, path);
        m_Models.emplace(id, model);
        return id;
    }

    Model *AssetManager::GetModel(ModelID id)
    {
        if (m_Models.find(id) != m_Models.end())
            return m_Models[id];
        DD_ERR("Trying to get model that doesn't exist! ID: {0}", id);
        return nullptr;
    }

    const std::string &AssetManager::GetModelPath(ModelID id)
    {
        if (m_ModelPath.find(id) != m_ModelPath.end())
        {
            return m_ModelPath[id];
        }
        DD_ERR("Trying to get path of model that doesn't exist! ID: {0}", id);
        static std::string empty = "";
        return empty;
    }

    Ref<Material> AssetManager::GetMaterial(const char *path)
    {
        if (m_MaterialID.find(path) != m_MaterialID.end())
        {

            return m_Materials[m_MaterialID.at(path)];
        }

        Ref<Material> mat = m_MaterialLoader->LoadMaterial(path);
        int id = m_Materials.size();
        m_MaterialID.emplace(path, id);
        m_Materials.emplace(id, mat);
        return mat;
    }
} // namespace Dodo