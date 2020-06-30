#pragma once

#include "Core/Common.h"

#include <glad/gl.h>

namespace Dodo {

	struct BufferElement {

		BufferElement(const char* name, const uint componentCount)
			: m_Name(name), m_ComponentCount(componentCount), m_Offset(0), m_Index(0)
		{}

		inline const uint GetComponentCount() const { return m_ComponentCount; }
		inline const char* GetName() const { return m_Name; }

	public:
		uchar m_Index;
		uchar m_Offset;
	private:
		const char* m_Name;
		const uchar m_ComponentCount;

	};

	struct BufferProperties
	{
		BufferProperties()
			: m_Stride(0)
		{}

		BufferProperties(std::initializer_list<BufferElement> elements)
			: m_Elements(elements), m_Stride(0)
		{
			for (int i = 0; i < m_Elements.size(); i++)
			{
				m_Elements[i].m_Index = i;
				m_Elements[i].m_Offset = m_Stride;
				m_Stride += m_Elements[i].GetComponentCount();
			}
		}

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

		uchar m_Stride;
		std::vector<BufferElement> m_Elements;
	private:
	};

	namespace Platform {


		class OpenGLVertexBuffer {
		private:
			uint m_BufferID;
		public:
			OpenGLVertexBuffer(const float* vertices, const uint& size, const BufferProperties& prop);
			~OpenGLVertexBuffer();

			void Create(const float* vertices, const uint& size);

			const BufferProperties& GetBufferProperties() const { return m_BufferProperties; }

			inline void Bind() const { glBindBuffer(GL_ARRAY_BUFFER, m_BufferID); }
		private:
			const BufferProperties m_BufferProperties;
		};

		class OpenGLIndexBuffer {
		private:
			uint m_BufferID;
		public:
			OpenGLIndexBuffer(const uint* indices, const uint& count);
			~OpenGLIndexBuffer();

			void Create(const uint* indices, const uint& count);

			inline void Bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID); }

			inline const uint GetCount() const { return m_Count; }
		private:
			const uint m_Count;
		};

		class OpenGLArrayBuffer {
		private:
			uint m_BufferID;
		public:
			OpenGLArrayBuffer(const OpenGLVertexBuffer* vBuffer, const OpenGLIndexBuffer* iBuffer);
			~OpenGLArrayBuffer();

			// No create function because of arraybuffer do not allocate much.

			inline void Bind() const { glBindVertexArray(m_BufferID); }

			inline const uint GetCount() const { return m_IndexBuffer->GetCount(); }
		private:
			const OpenGLVertexBuffer* m_VertexBuffer;
			const OpenGLIndexBuffer* m_IndexBuffer;
		};
	}

}