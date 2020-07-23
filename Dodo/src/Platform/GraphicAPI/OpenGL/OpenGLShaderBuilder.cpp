#include "pch.h"
#include <glad/gl.h>
#include "OpenGLShaderBuilder.h"
#include "Core/Application/Application.h"

namespace Dodo {
	namespace Platform {
		OpenGLShaderBuilder::OpenGLShaderBuilder()
		{
			InitFallbackShader();
		}

		void OpenGLShaderBuilder::InitFallbackShader()
		{
			const char* vertex =
				"#version 330 core\n"
				"layout (location = 0) in vec3 a_Position;\n"
				"layout (location = 1) in vec2 a_Texcoord;\n"
				"uniform mat4 u_Model = mat4(1.0f);\n"
				"uniform mat4 u_Camera = mat4(1.0f);\n"
				"out vec2 o_Texcoord;\n"
				"void main()\n"
				"{\n"
				"   o_Texcoord = a_Texcoord;\n"
				"   gl_Position = u_Camera * u_Model * vec4(a_Position, 1.0);\n"
				"}\0";

			const char* fragment =
				"#version 330 core\n"
				"in vec2 o_Texcoord;\n"
				"out vec4 pixel;\n"
				"float checker(vec2 uv)\n"
				"{\n"
				"	float cx = floor(20.0f * o_Texcoord.x);\n"
				"	float cy = floor(20.0f * o_Texcoord.y);\n"
				"	return sign(mod(cx + cy, 2.0f));\n"
				"}\n"
				"void main()\n"
				"{\n"
				"	float res = mix(1.0f, 0.0f, checker(o_Texcoord));\n"
				"   pixel = vec4(res, res * 0.1f, res * 0.1f, 1.0);\n"
				"}\0";

			m_FallbackShader = new Shader("FallbackShader", CompileVertexFragmentShader(vertex, fragment));
		}

		OpenGLShaderBuilder::~OpenGLShaderBuilder()
		{
			delete m_FallbackShader;
		}

		uint OpenGLShaderBuilder::CompileVertexFragmentShader(const char* vertex, const char* fragment)
		{
			uint result = glCreateProgram();

			// Vertex Shader
			uint vertexID = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertexID, 1, &vertex, NULL);
			glCompileShader(vertexID);

			int success;
			char infoLog[512];
			bool failed = false;
			glGetShaderiv(vertexID, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(vertexID, 512, NULL, infoLog);
				std::string stringlog = "Vertex Shader: ";
				stringlog.append(infoLog);
				stringlog = std::regex_replace(stringlog, std::regex("\\ERROR:"), "");
				DD_ERR(stringlog);
				failed = true;
			}

			// Fragment Shader

			uint fragmentID = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragmentID, 1, &fragment, NULL);
			glCompileShader(fragmentID);

			glGetShaderiv(fragmentID, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fragmentID, 512, NULL, infoLog);
				std::string stringlog = "Fragment Shader: ";
				stringlog.append(infoLog);
				stringlog = std::regex_replace(stringlog, std::regex("\\ERROR:"), "");
				DD_ERR(stringlog);
				failed = true;
			}


			if (failed)
			{
				Application::s_Application->m_Window->FocusConsole(); // Make the error more noticeable
				return -1; // Get error from both shaders
			}

			//LINKING

			glAttachShader(result, vertexID);
			glAttachShader(result, fragmentID);
			glLinkProgram(result);

			glGetProgramiv(result, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(result, 512, NULL, infoLog);
				std::string stringlog = infoLog;
				stringlog.append("Linking Shader: ");
				stringlog = std::regex_replace(stringlog, std::regex("\\Error:"), "");
				DD_ERR(stringlog);
				return -1;
			}
			glDeleteShader(vertexID);
			glDeleteShader(fragmentID);
			return result;
		}
	}
}