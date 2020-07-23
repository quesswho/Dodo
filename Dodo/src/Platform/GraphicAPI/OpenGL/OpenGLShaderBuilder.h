#pragma once

#include "Core/Common.h"
#include "Core/Graphics/Shader.h"

namespace Dodo {
	namespace Platform {

		class OpenGLShaderBuilder {
			Shader* m_FallbackShader;
		public:
			OpenGLShaderBuilder();
			~OpenGLShaderBuilder();

			uint CompileVertexFragmentShader(const char* vertex, const char* fragment);

			inline Shader* GetFallbackShader() const { return m_FallbackShader; }
		private:
			void InitFallbackShader();
		};
	}
}