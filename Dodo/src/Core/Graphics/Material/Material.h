#pragma once

#include "Core/Common.h"
#include "Core/Graphics/FrameBufferedDescriptorSet.h"
#include "Core/Graphics/Material/Texture.h"
#include "Core/Graphics/Material/TextureSampler.h"
#include "Core/Graphics/Pipeline/Pipeline.h"
#include "Core/Graphics/RenderAPI.h"
#include "Core/Utilities/Logger.h"

#include <vector>

namespace Dodo {
    class Material {
      public:
        Material();
        Material(Ref<Pipeline> shader);
        Material(Ref<Pipeline> shader, Ref<Texture> texture, Ref<TextureSampler> sampler);
        ~Material();

        void SetShader(Ref<Pipeline> shader)
        {
            m_Shader = shader;
            m_DescriptorSet.Reset();
        }
        Ref<Pipeline> GetShader() const { return m_Shader; }

        void AddTexture(uint slot, Ref<Texture> texture);
        Ref<Texture> GetTexture(uint slot) const
        {
            auto it = m_Textures.find(slot);
            if (it != m_Textures.end()) return it->second;
            DD_ERR("No texture at slot {} in material!", slot);
            return nullptr;
        }
        const std::unordered_map<uint, Ref<Texture>>& GetTextures() const { return m_Textures; }

        void SetSampler(Ref<TextureSampler> sampler)
        {
            m_Sampler = sampler;
            m_DescriptorSet.MarkDirty();
        }
        Ref<TextureSampler> GetSampler() const { return m_Sampler; }

        FrameBufferedDescriptorSet& GetDescriptorSet() const { return m_DescriptorSet; }

        void Bind(RenderAPI& renderAPI) const;

      private:
        Ref<Pipeline> m_Shader;
        std::unordered_map<uint, Ref<Texture>> m_Textures;
        Ref<TextureSampler> m_Sampler;
        mutable FrameBufferedDescriptorSet m_DescriptorSet;
    };
} // namespace Dodo