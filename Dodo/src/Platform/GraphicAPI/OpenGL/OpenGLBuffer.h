#pragma once

#include "Core/Graphics/BufferLayout.h"
#include <Core/Common.h>

namespace Dodo::Platform {
    class OpenGLVertexBuffer {
      private:
        uint m_VBufferID;
        uint m_ABufferID;

      public:
        OpenGLVertexBuffer(const float* vertices, const uint size, const BufferProperties& prop);
        ~OpenGLVertexBuffer();

        const BufferProperties& GetBufferProperties() const { return m_BufferProperties; }
        uint GetVAOID() const { return m_ABufferID; }

      private:
        const BufferProperties m_BufferProperties;
    };

    class OpenGLIndexBuffer {
      private:
        uint m_BufferID;

      public:
        OpenGLIndexBuffer(const uint* indices, const uint count);
        ~OpenGLIndexBuffer();

        void Bind() const;

        uint GetEBOID() const { return m_BufferID; }
        inline const uint GetCount() const { return m_Count; }

      private:
        const uint m_Count;
    };
} // namespace Dodo::Platform