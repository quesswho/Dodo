#pragma once
#include <vector>
#include "Core/Graphics/Texture.h"
#include "Core/Graphics/Shader.h"

namespace Dodo {

	class Material {
	private:
		std::vector<Texture*> m_Textures;

		Shader* m_Shader;
	public:
		Material();
		Material(Shader* shader, std::vector<Texture*> textures);

		~Material();

		inline void SetShader(Shader* shader) { m_Shader = shader; }
		void AddTexture(Texture* texture);

		void Bind();
	};
}