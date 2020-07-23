#pragma once
#include "Core/Common.h"

#include "OpenGLBuffer.h"

#include "Core/Math/Maths.h"
#include <unordered_map>

namespace Dodo {
	namespace Platform {

		class OpenGLShader {
		private:
			enum class ShaderType { UNKNOWN = -1, VERTEX = 0, FRAGMENT = 1 };

			uint m_ShaderID;
			const char* m_Name;
			std::unordered_map<const char*, int> m_UniformLocations;
		public:
			explicit OpenGLShader(const char* name, uint shader) // Prone to change
				: m_Name(name), m_ShaderID(shader)
			{}

			explicit OpenGLShader(const char* name, const char* path, const BufferProperties& shaderInput); // File path
			explicit OpenGLShader(const char* name, std::string& source, const BufferProperties& shaderInput); // Shader code

			~OpenGLShader();

			static OpenGLShader* CreateFromPath(const char* name, const char* path, const BufferProperties& shaderInput) { return new OpenGLShader(name, path, shaderInput); }
			static OpenGLShader* CreateFromSource(const char* name, std::string& source, const BufferProperties& shaderInput) { return new OpenGLShader(name, source, shaderInput); }

			void Bind() const;
			void Unbind() const;

			void ReloadFromPath(const char* path);
			void ReloadFromSource(std::string& source);

			void CreateConstantBuffers() {}

			const char* GetName() const { return m_Name; }

			void SetUniformValue(const char* location, const int value);
			void SetUniformValue(const char* location, const float value);
			void SetUniformValue(const char* location, const Math::TVec2<float>& value);
			void SetUniformValue(const char* location, const Math::TVec3<float>& value);
			void SetUniformValue(const char* location, const Math::TVec4<float>& value);
			void SetUniformValue(const char* location, const Math::Mat2& value);
			void SetUniformValue(const char* location, const Math::Mat3& value);
			void SetUniformValue(const char* location, const Math::Mat4& value);
		private:

			void CompileInit(std::string& fileSource);
			void CompileVFShader(const char* vertex, const char* fragment);

			int GetLocation(const char* location);
		};
	}
}