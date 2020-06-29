#include "pch.h"
#include "OpenGLBuffer.h"

#include "Core/Application/Application.h"

#include <glad/gl.h>

namespace Dodo {
	namespace Platform {

		OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, const uint size, const BufferProperties& prop)
			: m_BufferProperties(prop), m_BufferID(0)
		{
			Application::s_Application->m_ThreadManager->Task(std::bind(&OpenGLVertexBuffer::Create, this, vertices, size));
		}

		OpenGLVertexBuffer::~OpenGLVertexBuffer()
		{
			glDeleteBuffers(1, &m_BufferID);
		}

		inline void OpenGLVertexBuffer::Create(float* vertices, const uint size)
		{
			glGenBuffers(1, &m_BufferID);
			glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
			glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
		}

		inline void OpenGLVertexBuffer::Bind() const
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
		}

		OpenGLIndexBuffer::OpenGLIndexBuffer(uint* indices, const uint count)
			: m_Count(count)
		{
			Application::s_Application->m_ThreadManager->Task(std::bind(&OpenGLIndexBuffer::Create, this, indices, count));
		}

		OpenGLIndexBuffer::~OpenGLIndexBuffer()
		{
			glDeleteBuffers(1, &m_BufferID);
		}

		inline void OpenGLIndexBuffer::Create(uint* indices, const uint count)
		{
			glGenBuffers(1, &m_BufferID);
			glBindBuffer(GL_INDEX_ARRAY, m_BufferID);
			glBufferData(GL_INDEX_ARRAY, count * sizeof(uint), indices, GL_STATIC_DRAW);
		}

		inline void OpenGLIndexBuffer::Bind() const
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID);
		}
	}
}