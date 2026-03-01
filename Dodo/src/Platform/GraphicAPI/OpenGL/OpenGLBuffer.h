#pragma once

#include <Core/Common.h>

#include <glad/gl.h>

namespace Dodo {

    struct BufferElement {

        BufferElement(const char *name, const uint componentCount)
            : m_Name(name), m_ComponentCount(componentCount), m_Offset(0), m_Index(0)
        {}

        inline const uint GetComponentCount() const
        {
            return m_ComponentCount;
        }
        inline const char *GetEntryName() const
        {
            return m_Name;
        }

      public:
        uchar m_Index;
        uchar m_Offset;

      private:
        const char *m_Name;
        const uchar m_ComponentCount;
    };

    struct BufferProperties {
        BufferProperties() : m_Stride(0)
        {}

        BufferProperties(std::initializer_list<BufferElement> elements) : m_Elements(elements), m_Stride(0)
        {
            // TODO: if component count is greater than 4, divide it because of limitation in opengl
            for (int i = 0; i < m_Elements.size(); i++)
            {
                m_Elements[i].m_Index = i;
                m_Elements[i].m_Offset = m_Stride;
                m_Stride += m_Elements[i].GetComponentCount();
            }
        }

        std::vector<BufferElement>::iterator begin()
        {
            return m_Elements.begin();
        }
        std::vector<BufferElement>::iterator end()
        {
            return m_Elements.end();
        }
        std::vector<BufferElement>::const_iterator begin() const
        {
            return m_Elements.begin();
        }
        std::vector<BufferElement>::const_iterator end() const
        {
            return m_Elements.end();
        }

        uchar m_Stride;
        std::vector<BufferElement> m_Elements;

      private:
    };

    namespace Platform {

        class OpenGLVertexBuffer {
          private:
            uint m_VBufferID;
            uint m_ABufferID;

          public:
            OpenGLVertexBuffer(const float *vertices, const uint size, const BufferProperties &prop);
            ~OpenGLVertexBuffer();

            const BufferProperties &GetBufferProperties() const
            {
                return m_BufferProperties;
            }

            inline void Bind() const
            {
                glBindVertexArray(m_ABufferID);
            }

          private:
            const BufferProperties m_BufferProperties;
        };

        class OpenGLIndexBuffer {
          private:
            uint m_BufferID;

          public:
            OpenGLIndexBuffer(const uint *indices, const uint count);
            ~OpenGLIndexBuffer();

            inline void Bind() const
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID);
            }

            inline const uint GetCount() const
            {
                return m_Count;
            }

          private:
            const uint m_Count;
        };
    } // namespace Platform
} // namespace Dodo