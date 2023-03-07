#pragma once

#include "Core/Graphics/Buffer.h"
#include "Core/Graphics/CubeMap.h"
#include "Core/Graphics/Shader/Shader.h"
#include "Core/Graphics/Material/Texture.h"

#include "Core/Math/Matrix/Mat4.h"

namespace Dodo {

	class Skybox {
	private:
		VertexBuffer* m_VertexBuffer;
		Ref<CubeMapTexture> m_CubeMapTexture;
		Ref<Shader> m_Shader;
	public:
		Math::Mat4 m_Projection;
		Skybox(const Math::Mat4& projection, std::vector<std::string> paths);
		~Skybox();

		void Draw(const Math::Mat4& viewMatrix) const;
	};
}