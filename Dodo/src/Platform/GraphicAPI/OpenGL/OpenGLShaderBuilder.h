#pragma once

#include "Core/Common.h"
#include "Core/Graphics/Material/Shader.h"

namespace Dodo {

	enum ShaderBuilderFlags
	{
		ShaderBuilderFlagNone = 0,
		ShaderBuilderFlagNoTexcoord = 1 << 0,
		ShaderBuilderFlagTangentSpaceNormal = 1 << 1,
		ShaderBuilderFlagTangentSpace = 1 << 2,
		ShaderBuilderFlagColorUniform = 1 << 3,
		ShaderBuilderFlagsCameraPositionUniform = 1 << 4,
		ShaderBuilderFlagLightDirectionUniform = 1 << 5,
		ShaderBuilderFlagSpecularUniform = 1 << 6,
		ShaderBuilderFlagDiffuseMap = 1 << 7,
		ShaderBuilderFlagSpecularMap = 1 << 8,
		ShaderBuilderFlagNormalMap = 1 << 9,
		ShaderBuilderFlagCubeMap = 1 << 10,
		ShaderBuilderFlagMaxDepth = 1 << 11,
	};
#pragma warning(push)
#pragma warning(disable : 26812)
	DEFINE_ENUM_FLAG_OPERATORS(ShaderBuilderFlags);
#pragma warning(pop)

	namespace Platform {

		class OpenGLShaderBuilder {
			Shader* m_FallbackShader;
		public:
			OpenGLShaderBuilder();
			~OpenGLShaderBuilder();

			Shader* BuildVertexFragmentShader(const ShaderBuilderFlags flags, const char* name) const;
			inline Shader* BuildVertexFragmentShader(const ShaderBuilderFlags flags) const { return BuildVertexFragmentShader(flags, std::to_string(flags).c_str()); }

			uint CompileVertexFragmentShader(const char* vertex, const char* fragment) const;

			inline Shader* GetFallbackShader() const { return m_FallbackShader; }
		private:
			void InitFallbackShader();
		};
	}
}