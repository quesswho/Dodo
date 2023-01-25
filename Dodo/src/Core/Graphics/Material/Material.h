#pragma once
#include <vector>
#include "Texture.h"
#include "Shader.h"

namespace Dodo {

	class Material {
	private:
		std::vector<Texture*> m_Textures;

		Shader* m_Shader;
	public:
		Material();
		Material(Shader* shader);
		Material(Shader* shader, Texture* texture);
		Material(Shader* shader, std::vector<Texture*> textures);

		~Material();

		void AddTexture(Texture* texture);

		Texture* GetTexture(uint index)
		{
			if (m_Textures.size() > index)
				return m_Textures[index];
			
			DD_ERR("Texture index does not exist in material!");
		}

		template<typename T>
		void SetUniform(const char* location, T value) {
			m_Shader->Bind();
			m_Shader->SetUniformValue(location, value); 
		}

		void Bind();
	};
}