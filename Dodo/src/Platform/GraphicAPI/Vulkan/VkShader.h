#pragma once

#include <Core/Common.h>

#include "Core/Math/Maths.h"

namespace Dodo::Platform {

    class VkShader {
      public:
        VkShader(uint shader) : m_ShaderID(shader) {}

        ~VkShader();

        void Bind() const;
        void Unbind() const;

        void SetUniformValue(const char* location, const int value);
        void SetUniformValue(const char* location, const float value);
        void SetUniformValue(const char* location, const Math::TVec2<float>& value);
        void SetUniformValue(const char* location, const Math::TVec3<float>& value);
        void SetUniformValue(const char* location, const Math::TVec4<float>& value);
        void SetUniformValue(const char* location, const Math::Mat2& value);
        void SetUniformValue(const char* location, const Math::Mat3& value);
        void SetUniformValue(const char* location, const Math::Mat4& value);

      private:
        uint m_ShaderID;
    };
} // namespace Dodo::Platform