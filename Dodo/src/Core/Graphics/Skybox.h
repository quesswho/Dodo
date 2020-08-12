#pragma once
#include "Core/Graphics/Buffer.h"
#include "Core/Graphics/CubeMap.h"
#include "Core/Graphics/Material/Shader.h"
#include "Core/Math/Matrix/Mat4.h"
#include "Core/Graphics/Material/Texture.h"

namespace Dodo {

	class Skybox {
	private:
		VertexBuffer* m_VertexBuffer;
		CubeMapTexture* m_CubeMapTexture;
		Texture* m_Texture;
		Shader* m_Shader;
	public:
		Math::Mat4 m_Projection;
		Skybox(const Math::Mat4& projection, std::vector<std::string> paths);
		~Skybox();



		void Draw(const Math::Mat4& viewMatrix) const;
	};
}