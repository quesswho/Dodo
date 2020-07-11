#include "pch.h"
#include "OpenGLBuffer.h"

namespace Dodo {
	namespace Platform {

		// VertexBuffer //

		OpenGLVertexBuffer::OpenGLVertexBuffer(const float* vertices, const uint& size, const BufferProperties& prop)
			: m_BufferProperties(prop), m_BufferID(0)
		{
			glGenBuffers(1, &m_BufferID);
			Create(vertices, size);
		}

		OpenGLVertexBuffer::~OpenGLVertexBuffer()
		{
			glDeleteBuffers(1, &m_BufferID);
		}

		inline void OpenGLVertexBuffer::Create(const float* vertices, const uint& size)
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
			glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
		}

		// IndexBuffer //

		OpenGLIndexBuffer::OpenGLIndexBuffer(const uint* indices, const uint& count)
			: m_Count(count), m_BufferID(0)
		{
			glGenBuffers(1, &m_BufferID);
			Create(indices, count);
		}

		OpenGLIndexBuffer::~OpenGLIndexBuffer()
		{
			glDeleteBuffers(1, &m_BufferID);
		}

		inline void OpenGLIndexBuffer::Create(const uint* indices, const uint& count)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint), indices, GL_STATIC_DRAW);
		}

		// Array Buffer //

		OpenGLArrayBuffer::OpenGLArrayBuffer(const OpenGLVertexBuffer* vBuffer, const OpenGLIndexBuffer* iBuffer)
			: m_BufferID(0), m_VertexBuffer(vBuffer), m_IndexBuffer(iBuffer)
		{
			glGenVertexArrays(1, &m_BufferID);
			glBindVertexArray(m_BufferID);
			m_IndexBuffer->Bind();
			m_VertexBuffer->Bind();
			
			if (const uchar stride = m_VertexBuffer->GetBufferProperties().m_Stride; stride > 0)
			{
				for (const auto& element : m_VertexBuffer->GetBufferProperties())
				{
					glEnableVertexAttribArray(element.m_Index);
					glVertexAttribPointer(element.m_Index, element.GetComponentCount(), 
						GL_FLOAT, GL_FALSE, 
						stride * sizeof(float), 
						(const void*)(element.m_Offset * sizeof(float)));
				}
			}
		}

		OpenGLArrayBuffer::~OpenGLArrayBuffer()
		{
			glDeleteVertexArrays(1, &m_BufferID);
		}
	}
}