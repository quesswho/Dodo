#pragma once

#include "Core/Common.h"
#include "Core/Graphics/Material/Texture.h"
#include "Core/Graphics/Material/TextureSampler.h"
#include "Core/Graphics/RenderAPI.h"
#include "Core/Graphics/Shader/Shader.h"
#include "Core/Utilities/Logger.h"

#include <vector>

namespace Dodo {
    class Material {
      public:
        Material();
        Material(Ref<Shader> shader);
        Material(Ref<Shader> shader, Ref<Texture> texture,
                 Ref<TextureSampler> sampler = std::make_shared<TextureSampler>());
        ~Material();

        void SetShader(Ref<Shader> shader) { m_Shader = shader; }
        Ref<Shader> GetShader() const { return m_Shader; }

        void AddTexture(uint slot, Ref<Texture> texture);
        Ref<Texture> GetTexture(uint slot) const
        {
            auto it = m_Textures.find(slot);
            if (it != m_Textures.end()) return it->second;
            DD_ERR("No texture at slot {} in material!", slot);
            return nullptr;
        }
        const std::unordered_map<uint, Ref<Texture>>& GetTextures() const { return m_Textures; }

        void SetSampler(Ref<TextureSampler> sampler) { m_Sampler = sampler; }
        Ref<TextureSampler> GetSampler() const { return m_Sampler; }

        template <typename T>
        void SetUniform(const char* location, T value)
        {
            m_Shader->SetUniformValue(location, value);
        }

        void Bind(RenderAPI& renderAPI) const;

      private:
        Ref<Shader> m_Shader;
        std::unordered_map<uint, Ref<Texture>> m_Textures;
        Ref<TextureSampler> m_Sampler;
    };
} // namespace Dodo