#pragma once

#include <Core/Common.h>

#include "OpenGLBuffer.h"

#include "Core/Math/Maths.h"

#include <unordered_map>

namespace Dodo { namespace Platform {

    class OpenGLShader {
      private:
        enum class ShaderType
        {
            UNKNOWN = -1,
            VERTEX = 0,
            FRAGMENT = 1
        };

        uint m_ShaderID;
        const char *m_Name;
        std::unordered_map<std::string, int> m_UniformLocations;

      public:
        OpenGLShader(const char *name, uint shader) // Internal use only
            : m_Name(name), m_ShaderID(shader)
        {}

        OpenGLShader(const char *name, uint shader,
                     std::unordered_map<std::string, int> uniformLocations) // Internal use only
            : m_Name(name), m_ShaderID(shader), m_UniformLocations(uniformLocations)
        {}

        explicit OpenGLShader(const char *name, const char *path);    // File path
        explicit OpenGLShader(const char *name, std::string &source); // Shader code

        ~OpenGLShader();

        static Ref<OpenGLShader> CreateFromPath(const char *name, const char *path)
        {
            return std::make_shared<OpenGLShader>(name, path);
        }
        static Ref<OpenGLShader> CreateFromSource(const char *name, std::string &source)
        {
            return std::make_shared<OpenGLShader>(name, source);
        }

        void Bind() const;
        void Unbind() const;

        void ReloadFromPath(const char *path);
        void ReloadFromSource(std::string &source);
        void ReloadFromSource(const char *vertex, const char *fragment);

        void CreateConstantBuffers() {}

        const char *GetEntryName() const { return m_Name; }

        void SetUniformValue(const char *location, const int value);
        void SetUniformValue(const char *location, const float value);
        void SetUniformValue(const char *location, const Math::TVec2<float> &value);
        void SetUniformValue(const char *location, const Math::TVec3<float> &value);
        void SetUniformValue(const char *location, const Math::TVec4<float> &value);
        void SetUniformValue(const char *location, const Math::Mat2 &value);
        void SetUniformValue(const char *location, const Math::Mat3 &value);
        void SetUniformValue(const char *location, const Math::Mat4 &value);

      private:
        void CompileInit(const std::string &fileSource);
        void CompileVFShader(const char *vertex, const char *fragment);

        int GetLocation(const char *location);
    };
}} // namespace Dodo::Platform