#pragma once

#include "Core/Graphics/BufferLayout.h"
#include <Core/Common.h>

#include <glad/gl.h>

namespace Dodo::Platform {
    class OpenGLVertexBuffer {
      private:
        uint m_VBufferID;
        uint m_ABufferID;

      public:
        OpenGLVertexBuffer(const float* vertices, const uint size, const BufferProperties& prop);
        ~OpenGLVertexBuffer();

        const BufferProperties& GetBufferProperties() const { return m_BufferProperties; }

        inline void Bind() const { glBindVertexArray(m_ABufferID); }

      private:
        const BufferProperties m_BufferProperties;
    };

    class OpenGLIndexBuffer {
      private:
        uint m_BufferID;

      public:
        OpenGLIndexBuffer(const uint* indices, const uint count);
        ~OpenGLIndexBuffer();

        inline void Bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID); }

        inline const uint GetCount() const { return m_Count; }

      private:
        const uint m_Count;
    };
} // namespace Dodo::Platform