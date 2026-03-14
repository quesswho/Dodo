#pragma once

#include <Core/Common.h>

#include "Core/Graphics/BufferLayout.h"

namespace Dodo::Platform {
    class VkVertexBuffer {
      private:
        uint m_VBufferID;
        uint m_ABufferID;

      public:
        VkVertexBuffer(const float* vertices, const uint size, const BufferProperties& prop);
        ~VkVertexBuffer();

        const BufferProperties& GetBufferProperties() const { return m_BufferProperties; }

        inline void Bind() const {}

      private:
        const BufferProperties m_BufferProperties;
    };

    class VkIndexBuffer {
      public:
      VkIndexBuffer(const uint* indices, const uint count);
      ~VkIndexBuffer();
      
      inline void Bind() const {}
      
      inline const uint GetCount() const { return m_Count; }
      
      private:
        uint m_BufferID;
        const uint m_Count;
    };
} // namespace Dodo::Platform