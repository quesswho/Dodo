#pragma once

#include <Core/Common.h>

#include "AssetTypes.h"
#include "Core/Graphics/CubeMap.h"
#include "Core/Graphics/Pipeline/Pipeline.h"
#include "Core/Graphics/Pipeline/PipelineDesc.h"
#include "Core/Graphics/Pipeline/SlangCompiler.h"
#include "Core/Graphics/Pipeline/SlangGenerator.h"
#include "Core/Graphics/Scene/Mesh/MeshFactory.h"
#include "Core/Graphics/Scene/Model.h"
#include "MaterialLoader.h"
#include "ModelLoader.h"
#include "ShaderAsset.h"

namespace Dodo {

    enum class BuiltinModel {
        Cube,
    };

    class AssetManager {
      private:
        std::unordered_map<ShaderID, ShaderAsset> m_Shaders;
        std::unordered_map<std::string, ShaderID> m_ShaderPathLookup;
        std::unordered_map<ShaderBuilderFlags, ShaderID>
            m_ShaderBuilderShaders; // Stores all shaders created by shaderbuilder

        std::unordered_map<PipelineID, Ref<Pipeline>> m_Pipelines;
        std::unordered_map<ShaderBuilderFlags, PipelineID> m_ShaderBuilderPipelines;

        std::unordered_map<MaterialID, Ref<Material>> m_Materials;
        std::unordered_map<std::string, MaterialID> m_MaterialID;

        std::unordered_map<ModelID, Model*> m_Models;         // Stores id as key and model pointer as value
        std::unordered_map<std::string, ModelID> m_ModelID;   // Stores path as key and id as value
        std::unordered_map<ModelID, std::string> m_ModelPath; // Stores id as key and path as value
        std::unordered_map<BuiltinModel, ModelID> builtinIDs;

      public:
        AssetManager();
        ~AssetManager();

        ShaderID LoadShader(
            ShaderBuilderFlags flags,
            RenderAPI& renderAPI); // TODO: We will do something different here so that we do not need renderAPI
        ShaderID LoadShaderFromPath(const std::string& path);
        ShaderID LoadShader(SlangSource source);
        ShaderAsset& GetShaderAsset(ShaderID id); // This function is used when by the OpenGL backend to cache glsl code
        const ShaderAsset& GetShaderAsset(ShaderID id) const;

        PipelineID CreatePipeline(const PipelineDesc& desc, RenderAPI& renderAPI);
        PipelineID CreatePipeline(ShaderBuilderFlags flags, RenderAPI& renderAPI);
        Ref<Pipeline> GetPipeline(PipelineID id);
        Ref<Pipeline> GetFallbackPipeline() const { return m_Pipelines.at(0); }

        MaterialID LoadMaterial(const std::string& path);
        Ref<Material> GetMaterial(MaterialID id);

        ModelID LoadModel(const std::string& path);
        ModelID GetBuiltinModel(BuiltinModel type);
        Model* GetModel(ModelID id);

        std::string GetModelPath(ModelID id);
        bool HasPath(ModelID id) const { return m_ModelPath.find(id) != m_ModelPath.end(); }

      private:
        ShaderAsset SlangSourceToAsset(const SlangSource& source);

        ModelLoader m_ModelLoader;
        MaterialLoader m_MaterialLoader;
        MeshFactory m_MeshFactory;
        SlangCompiler m_SlangCompiler;

        ShaderID m_NextShaderID = 1;
        PipelineID m_NextPipelineID = 1;
        MaterialID m_NextMaterialID = 1;
        ModelID m_NextModelID = 1;
    };
} // namespace Dodo
