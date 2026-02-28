#pragma once

#include "Texture.h"
#include "Core/Graphics/Shader/Shader.h"

#include <vector>

namespace Dodo {

	class Material {
	private:
		std::vector<Ref<Texture>> m_Textures;

		Ref<Shader> m_Shader;
	public:
		Material();
		Material(Ref<Shader> shader);
		Material(Ref<Shader> shader, Ref<Texture> texture);
		Material(Ref<Shader> shader, std::vector<Ref<Texture>> textures);

		~Material();

		void AddTexture(Ref<Texture> texture);

		Ref<Texture> GetTexture(uint index)
		{
			if (m_Textures.size() > index)
				return m_Textures[index];
			
			DD_ERR("Texture index does not exist in material!");
		}

		template<typename T>
		void SetUniform(const char* location, T value) {
			m_Shader->SetUniformValue(location, value); 
		}

		void BindShader() const;
		void Bind() const;
	};
}