#include "pch.h"
#include "OpenGLBuffer.h"

namespace Dodo {
	namespace Platform {

		// VertexBuffer //

		OpenGLVertexBuffer::OpenGLVertexBuffer(const float* vertices, const uint& size, const BufferProperties& prop)
			: m_BufferProperties(prop), m_VBufferID(0)
		{
			glGenBuffers(1, &m_VBufferID);
			glBindBuffer(GL_ARRAY_BUFFER, m_VBufferID);
			glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

			glGenVertexArrays(1, &m_ABufferID);
			glBindVertexArray(m_ABufferID);

			glBindBuffer(GL_ARRAY_BUFFER, m_VBufferID);

			if (const uchar stride = m_BufferProperties.m_Stride; stride > 0)
			{
				for (const auto& element : m_BufferProperties.m_Elements)
				{
					glEnableVertexAttribArray(element.m_Index);
					glVertexAttribPointer(element.m_Index, element.GetComponentCount(),
						GL_FLOAT, GL_FALSE,
						stride * sizeof(float),
						(const void*)(element.m_Offset * sizeof(float)));
				}
			}
		}

		OpenGLVertexBuffer::~OpenGLVertexBuffer()
		{
			glDeleteBuffers(1, &m_VBufferID);
			glDeleteVertexArrays(1, &m_ABufferID);
		}

		// IndexBuffer //

		OpenGLIndexBuffer::OpenGLIndexBuffer(const uint* indices, const uint& count)
			: m_Count(count), m_BufferID(0)
		{
			glGenBuffers(1, &m_BufferID);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint), indices, GL_STATIC_DRAW);
		}

		OpenGLIndexBuffer::~OpenGLIndexBuffer()
		{
			glDeleteBuffers(1, &m_BufferID);
		}
	}
}