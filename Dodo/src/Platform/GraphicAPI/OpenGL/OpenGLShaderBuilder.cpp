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
				"float checker()\n"
				"{\n"
				"	float cx = floor(20.0f * o_Texcoord.x);\n"
				"	float cy = floor(20.0f * o_Texcoord.y);\n"
				"	return sign(mod(cx + cy, 2.0f));\n"
				"}\n"
				"void main()\n"
				"{\n"
				"	float res = mix(1.0f, 0.0f, checker());\n"
				"   pixel = vec4(res, res * 0.1f, res * 0.1f, 1.0);\n"
				"}\0";

			m_FallbackShader = new Shader("FallbackShader", CompileVertexFragmentShader(vertex, fragment));
		}

		OpenGLShaderBuilder::~OpenGLShaderBuilder()
		{
			delete m_FallbackShader;
		}

		Shader* OpenGLShaderBuilder::BuildVertexFragmentShader(const ShaderBuilderFlags flags, const char* name) const
		{
			////////////
			// Vertex //
			////////////

			std::string vertex = 
				"#version 330 core\n"
				"layout (location = 0) in vec3 a_Position;\n";

			// Buffer layout //

			if (flags | ShaderBuilderFlagNoTexcoord)
			{

				vertex.append("layout (location = 1) in vec2 a_Texcoord;\n");
				if (flags & ShaderBuilderFlagTangentSpace || flags & ShaderBuilderFlagNormalMap)
				{
					vertex.append("layout (location = 2) in vec3 a_Normal;\n");
					vertex.append("layout (location = 3) in vec3 a_Tangent;\n");
				}
				else
					vertex.append("layout (location = 2) in vec3 a_Normal;\n");
			}
			else
			{
				if (flags & ShaderBuilderFlagTangentSpace || flags & ShaderBuilderFlagNormalMap)
				{
					vertex.append("layout (location = 1) in vec3 a_Normal;\n");
					vertex.append("layout (location = 2) in vec3 a_Tangent;\n");
				}
				else
					vertex.append("layout (location = 1) in vec3 a_Normal;\n");
			}

			// Uniform //

			vertex.append(
				"uniform mat4 u_Model = mat4(1.0f);\n"
				"uniform mat4 u_Camera = mat4(1.0f);\n"
			);


			if (flags & ShaderBuilderFlagsCameraPositionUniform || flags & ShaderBuilderFlagSpecularUniform || flags & ShaderBuilderFlagDiffuseMap || flags & ShaderBuilderFlagSpecularMap || flags & ShaderBuilderFlagNormalMap)
			{
				vertex.append("uniform vec3 u_CameraPos = vec3(0.0f, 0.0f, 0.0f);\n");
			}

			if(flags & ShaderBuilderFlagLightDirectionUniform || flags & ShaderBuilderFlagSpecularUniform || flags & ShaderBuilderFlagDiffuseMap || flags & ShaderBuilderFlagSpecularMap || flags & ShaderBuilderFlagNormalMap)
			{
				vertex.append("uniform vec3 u_LightDir = vec3(0.0f, -0.5f, -0.5f);\n");
			}

			// Interface Block //

			vertex.append("out Vertex_Out {\n");
			if (flags | ShaderBuilderFlagNormalMap)
				vertex.append("		vec3 FragPos;\n");
			if (flags | ShaderBuilderFlagNoTexcoord)
				vertex.append("		vec2 TexCoord;\n");
			if (flags & ShaderBuilderFlagLightDirectionUniform || flags & ShaderBuilderFlagSpecularUniform || flags & ShaderBuilderFlagDiffuseMap || flags & ShaderBuilderFlagSpecularMap || flags & ShaderBuilderFlagNormalMap)
				vertex.append("		vec3 LightDirection;\n");
			if (flags & ShaderBuilderFlagNormalMap)
			{
				vertex.append(
					"		vec3 TangentCameraPos;\n"
					"		vec3 TangentFragPos;\n"
				);
			}
			else
			{
				vertex.append("		vec3 Normal;\n");
			}
			if (flags & ShaderBuilderFlagsCameraPositionUniform || flags & ShaderBuilderFlagSpecularUniform || flags & ShaderBuilderFlagDiffuseMap || flags & ShaderBuilderFlagSpecularMap || flags & ShaderBuilderFlagNormalMap)
				vertex.append("		vec3 CameraPos;\n");

			vertex.append(
				"} vertex_out;\n"
				"\n"
				"void main() {\n"
				"	vertex_out.FragPos = vec3(u_Model * vec4(a_Position, 1.0f));\n"
			);

			// Main //

			if (flags | ShaderBuilderFlagNoTexcoord)
				vertex.append("		vertex_out.TexCoord = a_Texcoord;\n");

			if (flags & ShaderBuilderFlagLightDirectionUniform || flags & ShaderBuilderFlagSpecularUniform || flags & ShaderBuilderFlagDiffuseMap || flags & ShaderBuilderFlagSpecularMap)
			{
				if(flags | ShaderBuilderFlagNormalMap)
					vertex.append("		vertex_out.LightDirection = normalize(-u_LightDir);\n");
			}

			if (flags & ShaderBuilderFlagNormalMap)
			{
				vertex.append(
					"	mat3 normalMatrix = transpose(inverse(mat3(u_Model)));\n"
					"	vec3 T = normalize(normalMatrix * a_Tangent);\n"
					"	vec3 N = normalize(normalMatrix * a_Normal);\n"
					"	T = normalize(T - dot(T, N) * N);\n"
					"	vec3 B = cross(N, T);\n"
					"	\n"
					"	mat3 TBN = transpose(mat3(T, B, N));\n"
					"	vertex_out.TangentCameraPos = TBN * u_CameraPos;\n"
					"	vertex_out.TangentFragPos = TBN * vertex_out.FragPos;\n"
					"	vertex_out.LightDirection = TBN * normalize(-u_LightDir);"
				);
			}
			else
			{
				vertex.append("		vertex_out.Normal = a_Normal;\n");
				if (flags & ShaderBuilderFlagsCameraPositionUniform || flags & ShaderBuilderFlagSpecularUniform || flags & ShaderBuilderFlagDiffuseMap || flags & ShaderBuilderFlagSpecularMap || flags & ShaderBuilderFlagNormalMap)
					vertex.append("		vertex_out.CameraPos = u_CameraPos;\n");
			}

			vertex.append("		gl_Position = u_Camera * u_Model * vec4(a_Position, 1.0);\n");
			vertex.append("}\0");

			//////////////
			// Fragment //
			//////////////

			std::string fragment =
				"#version 330 core\n"
				"out vec4 pixel;";

			// Uniform //

			if (flags & ShaderBuilderFlagSpecularUniform)
			{
				fragment.append("uniform float u_Specular = 0.0f;\n");
			}

			if(flags & ShaderBuilderFlagColorUniform)
				fragment.append("uniform vec3 u_Color = vec3(1.0f, 0.0f, 0.0f);\n");

			if (flags & ShaderBuilderFlagNormalMap)
				fragment.append("uniform sampler2D u_NormalMap;\n");
			if (flags & ShaderBuilderFlagDiffuseMap)
				fragment.append("uniform sampler2D u_DiffuseMap;\n");
			if (flags & ShaderBuilderFlagSpecularMap)
				fragment.append("uniform sampler2D u_SpecularMap;\n");

			// Interface Block //

			fragment.append("in Vertex_Out {\n");
			if(flags | ShaderBuilderFlagNormalMap)
				fragment.append("		vec3 FragPos;\n");
			if (flags | ShaderBuilderFlagNoTexcoord)
				fragment.append("		vec2 TexCoord;\n");
			if (flags & ShaderBuilderFlagLightDirectionUniform || flags & ShaderBuilderFlagSpecularUniform || flags & ShaderBuilderFlagDiffuseMap || flags & ShaderBuilderFlagSpecularMap || flags & ShaderBuilderFlagNormalMap)
				fragment.append("		vec3 LightDirection;\n");
			if (flags & ShaderBuilderFlagNormalMap)
			{
				fragment.append(
					"		vec3 TangentCameraPos;\n"
					"		vec3 TangentFragPos;\n"
				);
			}
			else
				fragment.append("		vec3 Normal;\n");

			if(flags & ShaderBuilderFlagsCameraPositionUniform || flags & ShaderBuilderFlagSpecularUniform || flags & ShaderBuilderFlagDiffuseMap || flags & ShaderBuilderFlagSpecularMap || flags & ShaderBuilderFlagNormalMap)
				fragment.append("		vec3 CameraPos;\n");

			fragment.append(
				"} frag_in;\n"
				"void main() {\n"
				"	vec3 result = vec3(0.0f);\n"
			);

			// Main //

			if (flags & ShaderBuilderFlagNormalMap)
				fragment.append("	vec3 normal = normalize(texture(u_NormalMap, frag_in.TexCoord).rgb * 2.0f - 1.0f);\n");
			else
				fragment.append("	vec3 normal = frag_in.Normal;\n");

			if (flags & ShaderBuilderFlagDiffuseMap)
				if (flags & ShaderBuilderFlagColorUniform)
					fragment.append("	vec3 color = texture(u_DiffuseMap, frag_in.TexCoord).rgb * u_Color;\n");
				else
					fragment.append("	vec3 color = texture(u_DiffuseMap, frag_in.TexCoord).rgb;\n");
			else
				if (flags & ShaderBuilderFlagColorUniform)
					fragment.append("	vec3 color = u_Color;\n");
				else
					fragment.append("	vec3 color = vec3(1.0f, 0.0f, 0.0f);\n");

			if (flags & ShaderBuilderFlagLightDirectionUniform || flags & ShaderBuilderFlagSpecularUniform || flags & ShaderBuilderFlagDiffuseMap || flags & ShaderBuilderFlagSpecularMap || flags & ShaderBuilderFlagNormalMap)
			{
				if (flags & ShaderBuilderFlagNormalMap)
				{
					fragment.append(
						"	vec3 ambient = color * 0.1f;\n"
						"	vec3 diffuse = max(dot(frag_in.LightDirection, normal), 0.0f) * color;\n"
						"	vec3 viewDir = normalize(frag_in.TangentCameraPos - frag_in.TangentFragPos);\n"
						"	vec3 halfwayDir = normalize(frag_in.LightDirection + viewDir);\n"
					);
					if (flags & ShaderBuilderFlagSpecularUniform)
						fragment.append("	vec3 specular = pow(max(dot(normal, halfwayDir), 0.0f), 32.0f) * vec3(u_Specular);\n");
					else if (flags & ShaderBuilderFlagSpecularMap)
						fragment.append("	vec3 specular = pow(max(dot(normal, halfwayDir), 0.0f), 32.0f) * texture(u_SpecularMap, frag_in.TexCoord).rgb;\n");
					fragment.append("	result = ambient + diffuse + specular;\n");
				}
				else
				{
					fragment.append(
						"	vec3 ambient = color * 0.1f;\n"
						"	vec3 diffuse = max(dot(frag_in.LightDirection, normal), 0.0f) * color;\n"
						"	vec3 viewDir = normalize(frag_in.CameraPos - frag_in.FragPos);\n"
						"	vec3 halfwayDir = normalize(frag_in.LightDirection + viewDir);\n"
					);

					if (flags & ShaderBuilderFlagSpecularUniform)
						fragment.append("	vec3 specular = pow(max(dot(normal, halfwayDir), 0.0f), 32.0f) * vec3(u_Specular);\n");
					else if(flags & ShaderBuilderFlagSpecularMap)
						fragment.append("	vec3 specular = pow(max(dot(normal, halfwayDir), 0.0f), 32.0f) * texture(u_SpecularMap, frag_in.TexCoord).rgb;\n");
					fragment.append("	result = ambient + diffuse + specular;\n");
				}
			}
			else
				fragment.append("	result = color;\n");
			fragment.append("	pixel = vec4(result, 1.0f);\n");

			fragment.append("}\0");
			Shader* shader = new Shader(std::to_string(flags).c_str(), CompileVertexFragmentShader(vertex.c_str(), fragment.c_str()));
			shader->Bind();
			if (flags & ShaderBuilderFlagDiffuseMap)
				shader->SetUniformValue("u_DiffuseMap", 0);
			if (flags & ShaderBuilderFlagSpecularMap)
			{
				shader->SetUniformValue("u_SpecularMap", 1);
				if (flags & ShaderBuilderFlagNormalMap)
					shader->SetUniformValue("u_NormalMap", 2);
			}
			else if (flags & ShaderBuilderFlagNormalMap)
				shader->SetUniformValue("u_NormalMap", 1);
			
			return shader;
		}

		uint OpenGLShaderBuilder::CompileVertexFragmentShader(const char* vertex, const char* fragment) const
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