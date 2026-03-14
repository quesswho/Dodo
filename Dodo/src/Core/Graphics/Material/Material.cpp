#include "Material.h"
#include "pch.h"

#include "Core/Application/Application.h"

namespace Dodo {

    Material::Material() : m_Shader(Application::s_Application->m_AssetManager->GetFallbackShader()) {}

    Material::Material(Ref<Shader> shader) : m_Shader(shader) {}

    Material::Material(Ref<Shader> shader, Ref<Texture> texture, Ref<TextureSampler> sampler)
        : m_Shader(shader), m_Sampler(sampler)
    {
        m_Textures[0] = texture;
    }

    Material::~Material() {}

    void Material::AddTexture(uint slot, Ref<Texture> texture)
    {
        if (m_Textures.count(slot)) {
            DD_WARN("Overwriting texture at slot {}", slot);
        }
        m_Textures[slot] = texture;
    }

    void Material::Bind(RenderAPI& renderAPI) const
    {
        m_Shader->Bind();
        for (const auto& [slot, texture] : m_Textures) {
            renderAPI.BindTexture(slot, texture);
            renderAPI.BindTextureSampler(slot, m_Sampler);
        }
    }
} // namespace Dodo