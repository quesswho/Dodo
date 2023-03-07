#include "pch.h"
#include "OpenGLShader.h"

#include "Core/System/FileUtils.h"
#include "Core/Application/Application.h"

#include <glad/gl.h>

namespace Dodo {
	namespace Platform {

		OpenGLShader::OpenGLShader(const char* name, const char* path)
			: m_Name(name)
		{
			size_t len = strlen(path);
			if (path[len - 1] == 'x' && path[len - 2] == '.') // Why is this here?
			{
				len += strlen("lsl");
				char* newpath = new char[len + 1];
				*newpath = '\0';
				strcat(newpath, path);
				newpath[strlen(path) - 1] = 'g';
				CompileInit(FileUtils::ReadTextFile(strcat(newpath, "lsl"))); // turning .x into .glsl
				delete[] newpath;
			}
			else
			{
				CompileInit(FileUtils::ReadTextFile(path));
			}
		}

		OpenGLShader::OpenGLShader(const char* name, std::string& source)
			: m_Name(name)
		{
			CompileInit(source);
		}

		OpenGLShader::~OpenGLShader()
		{
			glDeleteProgram(m_ShaderID);
		}

		void OpenGLShader::CompileVFShader(const char* vertex, const char* fragment)
		{
			m_ShaderID = glCreateProgram();

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
				stringlog = std::regex_replace(stringlog, std::regex("\\ERROR:"), m_Name);
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
				stringlog = std::regex_replace(stringlog, std::regex("\\ERROR:"), m_Name);
				DD_ERR(stringlog);
				failed = true;
			}


			if (failed)
			{
				Application::s_Application->m_Window->FocusConsole(); // Make the error more noticeable
				return; // Get error from both shaders
			}

			//LINKING
			
			glAttachShader(m_ShaderID, vertexID);
			glAttachShader(m_ShaderID, fragmentID);
			glLinkProgram(m_ShaderID);

			glGetProgramiv(m_ShaderID, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(m_ShaderID, 512, NULL, infoLog);
				std::string stringlog = infoLog;
				stringlog.append("Linking Shader: ");
				stringlog = std::regex_replace(stringlog, std::regex("\\Error:"), m_Name);
				DD_ERR(stringlog);
				return;
			}
			glDeleteShader(vertexID);
			glDeleteShader(fragmentID);
		}


		void OpenGLShader::CompileInit(std::string& fileSource)
		{
			if (fileSource == "-1") return;
			ShaderType type = ShaderType::UNKNOWN;

			std::string stringFragmentSource = "";
			std::string stringVertexSource = "";

			std::istringstream source(fileSource);
			std::string line;
			while (std::getline(source, line))
			{
				// Trim beginning of line
				const auto strBegin = line.find_first_not_of(" \t");
				if(strBegin != std::string::npos)
					line = line.substr(strBegin);


				// Check for type
				if (line == "#shader fragment")
				{
					type = ShaderType::FRAGMENT;
					continue;
				}

				if (line == "#shader vertex")
				{
					type = ShaderType::VERTEX;
					continue;
				}

				switch (type) {
					case ShaderType::FRAGMENT:
						stringFragmentSource.append(line).append("\n");
						break;
					case ShaderType::VERTEX:
						stringVertexSource.append(line).append("\n");
						break;
				}
			}

			if (stringFragmentSource == "" || stringVertexSource == "")
			{
				DD_ERR("{0}: Source needs to be in a specific format! Add \"#shader fragment\" or \"#shader vertex\" as the first line of the different shaders to differentiate between between the differnt shader types!", m_Name);
				Application::s_Application->m_Window->FocusConsole();
				return;
			}

			CompileVFShader(stringVertexSource.c_str(), stringFragmentSource.c_str());
		}


		void OpenGLShader::ReloadFromPath(const char* path)
		{
			glDeleteProgram(m_ShaderID);
			CompileInit(FileUtils::ReadTextFile(path));
		}

		void OpenGLShader::ReloadFromSource(std::string& source)
		{
			glDeleteProgram(m_ShaderID);
			CompileInit(source);
		}

		void OpenGLShader::ReloadFromSource(const char* vertex, const char* fragment)
		{
			glDeleteProgram(m_ShaderID);
			CompileVFShader(vertex, fragment);
		}

		void OpenGLShader::Bind() const
		{
			glUseProgram(m_ShaderID);
		}

		void OpenGLShader::Unbind() const
		{
			glUseProgram(0);
		}


		int OpenGLShader::GetLocation(const char* location)
		{
			if (m_UniformLocations.find(location) != m_UniformLocations.end())
				return m_UniformLocations[location];

			// New location
			const int locationi = glGetUniformLocation(m_ShaderID, location);
			m_UniformLocations[location] = locationi;
			return locationi;
		}

		void OpenGLShader::SetUniformValue(const char* location, const int value)
		{
			glUniform1i(GetLocation(location), value);
		}

		void OpenGLShader::SetUniformValue(const char* location, const float value)
		{
			glUniform1f(GetLocation(location), value);
		}
		void OpenGLShader::SetUniformValue(const char* location, const Math::TVec2<float>& value)
		{
			glUniform2f(GetLocation(location), value.x, value.y);
		}
		void OpenGLShader::SetUniformValue(const char* location, const Math::TVec3<float>& value)
		{
			glUniform3f(GetLocation(location), value.x, value.y, value.z);
		}
		void OpenGLShader::SetUniformValue(const char* location, const Math::TVec4<float>& value)
		{
			glUniform4f(GetLocation(location), value.x, value.y, value.z, value.w);
		}
		void OpenGLShader::SetUniformValue(const char* location, const Math::Mat2& value)
		{
			glUniformMatrix2fv(GetLocation(location), 1, GL_FALSE, &value.m_Elements[0]);
		}
		void OpenGLShader::SetUniformValue(const char* location, const Math::Mat3& value)
		{
			glUniformMatrix3fv(GetLocation(location), 1, GL_FALSE, &value.m_Elements[0]);
		}
		void OpenGLShader::SetUniformValue(const char* location, const Math::Mat4& value)
		{
			glUniformMatrix4fv(GetLocation(location), 1, GL_FALSE, &value.m_Elements[0]);
		}
	}
}