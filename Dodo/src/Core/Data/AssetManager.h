#pragma once

#include <Core/Common.h>

#include "Core/Graphics/CubeMap.h"
#include "Core/Graphics/Scene/Mesh/MeshFactory.h"
#include "Core/Graphics/Scene/Model.h"
#include "Core/Graphics/Shader/Shader.h"
#include "Core/Graphics/Shader/ShaderGenerator.h"
#include "Core/Graphics/Shader/SlangCompiler.h"
#include "MaterialLoader.h"
#include "ModelLoader.h"

namespace Dodo {

    using MaterialID = uint64_t;
    using ShaderID = uint64_t;

    enum class BuiltinModel {
        Cube,
    };

    class AssetManager {
      private:
        std::unordered_map<ShaderBuilderFlags, ShaderID>
            m_ShaderBuilderShaders; // Stores all shaders created by shaderbuilder

        std::unordered_map<ShaderID, Ref<Shader>> m_Shaders;
        std::unordered_map<std::string, ShaderID> m_ShaderPathLookup;

        std::unordered_map<MaterialID, Ref<Material>> m_Materials;
        std::unordered_map<std::string, MaterialID> m_MaterialID;

        std::unordered_map<ModelID, Model*> m_Models;         // Stores id as key and model pointer as value
        std::unordered_map<std::string, ModelID> m_ModelID;   // Stores path as key and id as value
        std::unordered_map<ModelID, std::string> m_ModelPath; // Stores id as key and path as value
        std::unordered_map<BuiltinModel, ModelID> builtinIDs;

      public:
        AssetManager();
        ~AssetManager();

        inline Ref<Shader> GetFallbackShader() const { return m_Shaders.at(0); }
        ShaderID LoadShader(ShaderBuilderFlags flags);
        ShaderID LoadShaderFromPath(const std::string& path);
        ShaderID LoadShader(ShaderSource source);
        Ref<Shader> GetShader(ShaderID id);

        MaterialID LoadMaterial(const std::string& path);
        Ref<Material> GetMaterial(MaterialID id);

        ModelID LoadModel(const std::string& path);
        ModelID GetBuiltinModel(BuiltinModel type);
        Model* GetModel(ModelID id);

        std::string GetModelPath(ModelID id);
        bool HasPath(ModelID id) const { return m_ModelPath.find(id) != m_ModelPath.end(); }

      private:
        ModelLoader m_ModelLoader;
        MaterialLoader m_MaterialLoader;
        MeshFactory m_MeshFactory;
        SlangCompiler m_SlangCompiler;

        ShaderID m_NextShaderID = 1;
        MaterialID m_NextMaterialID = 1;
        ModelID m_NextModelID = 1;
    };
} // namespace Dodo