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
			vertex.reserve(vertex.capacity() + 263); // atleast 263 characters always present in vertex shader

			// Buffer layout //

			if (!(flags & ShaderBuilderFlagNoTexcoord))
			{
				if (flags & ShaderBuilderFlagCubeMap)
					vertex.append("layout (location = 1) in vec3 a_Texcoord;\n");
				else
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
				else if(!(flags & ShaderBuilderFlagCubeMap))
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
				vertex.append("uniform vec3 u_LightDir = vec3(0.2f, -0.5f, -0.5f);\n");
			}

			if (flags & ShaderBuilderFlagShadowMap) {
				vertex.append("		uniform mat4 u_LightCamera;\n");
			}

			// Interface Block //

			vertex.append("out Vertex_Out {\n"
			"		vec3 FragPos;\n");
			if (!(flags & ShaderBuilderFlagNoTexcoord))
				if (flags & ShaderBuilderFlagCubeMap)
					vertex.append("		vec3 TexCoord;\n");
				else
					vertex.append("		vec2 TexCoord;\n");
			else if(flags & ShaderBuilderFlagCubeMap)
				vertex.append("		vec3 TexCoord;\n");

			if (flags & ShaderBuilderFlagLightDirectionUniform || flags & ShaderBuilderFlagSpecularUniform || flags & ShaderBuilderFlagDiffuseMap || flags & ShaderBuilderFlagSpecularMap || flags & ShaderBuilderFlagNormalMap)
				vertex.append("		vec3 LightDirection;\n");

			if (flags & ShaderBuilderFlagNormalMap)
			{
				vertex.append(
					"		vec3 TangentCameraPos;\n"
					"		vec3 TangentFragPos;\n"
				);
			}
			else if (!(flags & ShaderBuilderFlagCubeMap))
			{
				vertex.append("		vec3 Normal;\n");
			}
			if (flags & ShaderBuilderFlagsCameraPositionUniform || flags & ShaderBuilderFlagSpecularUniform || flags & ShaderBuilderFlagDiffuseMap || flags & ShaderBuilderFlagSpecularMap || flags & ShaderBuilderFlagNormalMap)
				vertex.append("		vec3 CameraPos;\n");
			if (flags & ShaderBuilderFlagShadowMap) {
				vertex.append("		vec4 LightFragPos;\n");
			}

			vertex.append(
				"} vertex_out;\n"
				"\n"
				"void main() {\n"
				"	vertex_out.FragPos = vec3(u_Model * vec4(a_Position, 1.0f));\n"
			);

			// Main //

			if (!(flags & ShaderBuilderFlagNoTexcoord))
				vertex.append("		vertex_out.TexCoord = a_Texcoord;\n");
			else if(flags & ShaderBuilderFlagCubeMap) // No texcoord & with cubemap
				vertex.append("		vertex_out.TexCoord = a_Position;\n");
			if (flags & ShaderBuilderFlagLightDirectionUniform || flags & ShaderBuilderFlagSpecularUniform || flags & ShaderBuilderFlagDiffuseMap || flags & ShaderBuilderFlagSpecularMap)
			{
				if(!(flags & ShaderBuilderFlagNormalMap))
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
					"	mat3 TBN = transpose(mat3(T, B, N));\n"
					"	vertex_out.TangentCameraPos = TBN * u_CameraPos;\n"
					"	vertex_out.TangentFragPos = TBN * vertex_out.FragPos;\n"
					"	vertex_out.LightDirection = TBN * normalize(-u_LightDir);\n"
				);
			}
			else if (!(flags & ShaderBuilderFlagCubeMap))
			{
				vertex.append("		vertex_out.Normal = a_Normal;\n");
				if (flags & ShaderBuilderFlagsCameraPositionUniform || flags & ShaderBuilderFlagSpecularUniform || flags & ShaderBuilderFlagDiffuseMap || flags & ShaderBuilderFlagSpecularMap || flags & ShaderBuilderFlagNormalMap)
					vertex.append("		vertex_out.CameraPos = u_CameraPos;\n");
			}
			if (flags & ShaderBuilderFlagShadowMap) {
				vertex.append("		vertex_out.LightFragPos = u_LightCamera * u_Model * vec4(a_Position, 1.0);\n");
			}
			if (flags & ShaderBuilderFlagMaxDepth)
			{
				vertex.append("		gl_Position = vec4(u_Camera * u_Model * vec4(a_Position, 1.0f)).xyww;\n");
			}
			else
			{
				vertex.append("		gl_Position = u_Camera * u_Model * vec4(a_Position, 1.0f);\n");
			}
			vertex.append("}\0");

			//////////////
			// Fragment //
			//////////////

			std::string fragment =
				"#version 330 core\n"
				"out vec4 pixel;\n";

			fragment.reserve(fragment.capacity() + 102); // atleast 102 characters always present in fragment shader
			// Uniform //

			if (flags & ShaderBuilderFlagSpecularUniform)
				fragment.append("uniform float u_Specular = 0.0f;\n");
			if(flags & ShaderBuilderFlagColorUniform)
				fragment.append("uniform vec3 u_Color = vec3(1.0f, 0.0f, 0.0f);\n");
			if (flags & ShaderBuilderFlagNormalMap)
				fragment.append("uniform sampler2D u_NormalMap;\n");
			if (flags & ShaderBuilderFlagDiffuseMap)
				fragment.append("uniform sampler2D u_DiffuseMap;\n");
			if (flags & ShaderBuilderFlagSpecularMap)
				fragment.append("uniform sampler2D u_SpecularMap;\n");
			if (flags & ShaderBuilderFlagCubeMap)
				fragment.append("uniform samplerCube u_CubeMap;\n");
			else if(flags & ShaderBuilderFlagBasicTexture)
				fragment.append("uniform sampler2D u_TextureMap;\n");
			if (flags & ShaderBuilderFlagShadowMap) {
				fragment.append("uniform sampler2D u_DepthMap;\n");
				fragment.append("float ShadowCalculation(vec3 normal);\n");
			}
			// Interface Block //

			fragment.append("in Vertex_Out {\n");
			fragment.append("		vec3 FragPos;\n");
			if (!(flags & ShaderBuilderFlagNoTexcoord))
				if(flags & ShaderBuilderFlagCubeMap)
					fragment.append("		vec3 TexCoord;\n");
				else
					fragment.append("		vec2 TexCoord;\n");
			else if (flags & ShaderBuilderFlagCubeMap)
				fragment.append("		vec3 TexCoord;\n");
			if (flags & ShaderBuilderFlagLightDirectionUniform || flags & ShaderBuilderFlagSpecularUniform || flags & ShaderBuilderFlagDiffuseMap || flags & ShaderBuilderFlagSpecularMap || flags & ShaderBuilderFlagNormalMap)
				fragment.append("		vec3 LightDirection;\n");
			if (flags & ShaderBuilderFlagNormalMap)
			{
				fragment.append(
					"		vec3 TangentCameraPos;\n"
					"		vec3 TangentFragPos;\n"
				);
			}
			else if (!(flags & ShaderBuilderFlagCubeMap))
				fragment.append("		vec3 Normal;\n");

			if(flags & ShaderBuilderFlagsCameraPositionUniform || flags & ShaderBuilderFlagSpecularUniform || flags & ShaderBuilderFlagDiffuseMap || flags & ShaderBuilderFlagSpecularMap || flags & ShaderBuilderFlagNormalMap)
				fragment.append("		vec3 CameraPos;\n");
			if (flags & ShaderBuilderFlagShadowMap) {
				fragment.append("		vec4 LightFragPos;\n");
			}

			fragment.append(
				"} frag_in;\n"
				"void main() {\n"
				"	vec4 result = vec4(0.0f);\n"
			);
			//if (flags & ShaderBuilderFlagShadowMap)
			//	fragment.append("pixel = vec4(vec3(texture(u_DepthMap, frag_in.TexCoord.xy).r), 1.0f);\n return;\n");

			// Main //
			if (flags & ShaderBuilderFlagCubeMap)
				fragment.append("	result = texture(u_CubeMap, frag_in.TexCoord.xyz).rgba;\n");
			if (flags & ShaderBuilderFlagNormalMap)
				fragment.append("	vec3 normal = normalize(texture(u_NormalMap, frag_in.TexCoord.xy).rgb * 2.0f - 1.0f);\n");
			else if(!(flags& ShaderBuilderFlagCubeMap))
				fragment.append("	vec3 normal = frag_in.Normal;\n");

			if (flags & ShaderBuilderFlagDiffuseMap) { // What will the color be assigned to
				if (flags & ShaderBuilderFlagColorUniform) {
					fragment.append("	vec4 color = vec4(texture(u_DiffuseMap, frag_in.TexCoord.xy).rgba * vec4(u_Color, 1.0f);\n");
				}
				else {
					fragment.append("	vec4 color = texture(u_DiffuseMap, frag_in.TexCoord.xy).rgba;\n");
				}
				fragment.append("	if(color.a < 0.85) discard;\n");
			}
			else if (!(flags & ShaderBuilderFlagCubeMap)) {
				if (flags & ShaderBuilderFlagColorUniform)
					fragment.append("	vec4 color = vec4(u_Color, 1.0f);\n");
				else if (flags & ShaderBuilderFlagBasicTexture)
					fragment.append("	vec4 color = texture(u_TextureMap, frag_in.TexCoord.xy).rgba;\n");
				else
					fragment.append("	vec4 color = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n");
			}

			if (flags & ShaderBuilderFlagShadowMap) {
				fragment.append("	float shadow = ShadowCalculation(normal);\n");
			}
			else {
				fragment.append("	float shadow = 0.0;\n");
			}

			if (flags & ShaderBuilderFlagLightDirectionUniform || flags & ShaderBuilderFlagSpecularUniform || flags & ShaderBuilderFlagDiffuseMap || flags & ShaderBuilderFlagSpecularMap || flags & ShaderBuilderFlagNormalMap)
			{
				if (flags & ShaderBuilderFlagNormalMap)
				{
					fragment.append(
						"	vec3 ambient = color.rgb * 0.1f;\n"
						"	vec4 diffuse = vec4(max(dot(frag_in.LightDirection, normal), 0.0f) * color.rgb, color.a);\n"
						"	vec3 viewDir = normalize(frag_in.TangentCameraPos - frag_in.TangentFragPos);\n"
						"	vec3 halfwayDir = normalize(frag_in.LightDirection + viewDir);\n"
					);
					if (flags & ShaderBuilderFlagSpecularUniform)
						fragment.append("	vec3 specular = pow(max(dot(normal, halfwayDir), 0.0f), 32.0f) * vec3(u_Specular);\n");
					else if (flags & ShaderBuilderFlagSpecularMap)
						fragment.append("	vec3 specular = pow(max(dot(normal, halfwayDir), 0.0f), 32.0f) * texture(u_SpecularMap, frag_in.TexCoord.xy).rrr;\n");
					if (flags & ShaderBuilderFlagSpecularMap || flags & ShaderBuilderFlagSpecularUniform) {
						fragment.append("	result = vec4(ambient + (1.0f - shadow) * (diffuse.rgb + specular), diffuse.a);\n");
					}
					else
					{
						fragment.append("	result = vec4(ambient + (1.0f - shadow) * (diffuse.rgb), diffuse.a);\n");
					}
				}
				else
				{
					fragment.append(
						"	vec3 ambient = color.rgb * 0.1f;\n"
						"	vec4 diffuse = vec4(max(dot(frag_in.LightDirection, normal), 0.0f) * color.rgb, color.a);\n"
						"	vec3 viewDir = normalize(frag_in.CameraPos - frag_in.FragPos);\n"
						"	vec3 halfwayDir = normalize(frag_in.LightDirection + viewDir);\n"
					);

					if (flags & ShaderBuilderFlagSpecularUniform)
					{
						fragment.append("	vec4 specular = pow(max(dot(normal, halfwayDir), 0.0f), 32.0f) * vec3(u_Specular);\n"
										"	result = vec4(ambient + (1.0f - shadow) * (diffuse.rgb + specular), diffuse.a);\n");

					}
					else if (flags & ShaderBuilderFlagSpecularMap)
					{
						fragment.append("	vec3 specular = pow(max(dot(normal, halfwayDir), 0.0f), 32.0f) * texture(u_SpecularMap, frag_in.TexCoord.xy).rrr;\n"
										"	result = vec4(ambient + (1.0f - shadow) * (diffuse.rgb + specular), diffuse.a);\n");
					}
					else
					{
						fragment.append("	result = vec4(ambient + (1.0f - shadow) * (diffuse.rgb), diffuse.a);\n");
					}
				}
			}
			else if (!(flags & ShaderBuilderFlagCubeMap))
				fragment.append("	result = color;\n"
								"	if(result.w < 0.1f) discard;\n");

			fragment.append(
				"	pixel = result;\n"
				"}\n");
			if (flags & ShaderBuilderFlagShadowMap) {
				/*fragment.append(
					"float ShadowCalculation(vec3 normal) {\n"
					"	vec3 projCoords = frag_in.LightFragPos.xyz / frag_in.LightFragPos.w;\n"
					"	if(projCoords.z > 1.0)\n"
					"		return 0.0;\n"
					"	projCoords = projCoords * 0.5 + 0.5;\n"
					"	float closestDepth = texture(u_DepthMap, projCoords.xy).r;\n"
					"	float currentDepth = projCoords.z;\n"
					"	float bias = max(0.07 * (1.0 - dot(normal, frag_in.LightDirection)), 0.01);\n"
					"	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;\n"
					"	return shadow;\n"
					"}\n");*/

				fragment.append(
					"vec2 sampleOffsetDirections[8] = vec2[](\n"
					"	vec2(1, 1), vec2(1, -1), vec2(-1, -1), vec2(-1, 1),\n"
					"	vec2(1, 0), vec2(-1, 0), vec2(0, 1), vec2(0, -1));\n"
					"float ShadowCalculation(vec3 normal) {\n"
					"	vec3 projCoords = frag_in.LightFragPos.xyz / frag_in.LightFragPos.w;\n"
					"	if(projCoords.z > 1.0)\n"
					"		return 0.0;\n"
					"	projCoords = projCoords * 0.5 + 0.5;\n"
					"	float currentDepth = projCoords.z;\n"
					"	float bias = max(0.005 * (1.0 - dot(normal, frag_in.LightDirection)), 0.001);\n"
					"	float shadow = 0.0;\n"
					"	int samples = 8;\n"
					"	vec2 texelSize = 1.0 / textureSize(u_DepthMap, 0);\n"
					"	float delta = 2.0/float(samples);"
					"	for (float x = -1; x <= 1-delta; x += delta)\n"
					"	{\n"
					"		for (float y = -1; y <= 1-delta; y += delta)\n"
					"		{\n"
					"			float closestDepth = texture(u_DepthMap, projCoords.xy + vec2(x,y) * texelSize).r;\n"
					"			if (currentDepth - bias > closestDepth)\n"
					"				shadow += 1.0;\n"
					"		}\n"
					"	}\n"
					"	shadow /= float(samples*samples);\n"
					"	return max(shadow, 0.0f);\n"
					"}\n");
			}
			fragment.append("\0");

			uint shaderid = CompileVertexFragmentShader(vertex.c_str(), fragment.c_str());
			if (!shaderid) {
				DD_ERR("Failed to build shader with flag {}", flags);
				DD_ERR("Vertex Shader: \n{}", vertex.c_str());
				DD_ERR("Fragment Shader: \n{}", fragment.c_str());
				return m_FallbackShader;
			}

			Shader* shader = new Shader(std::to_string(flags).c_str(), shaderid);
			shader->Bind();
			int i = 0;
			if (flags & ShaderBuilderFlagCubeMap)
				shader->SetUniformValue("u_CubeMap", i++);
			if (flags & ShaderBuilderFlagDiffuseMap)
				shader->SetUniformValue("u_DiffuseMap", i++);
			if (flags & ShaderBuilderFlagSpecularMap)
				shader->SetUniformValue("u_SpecularMap", i++);
			if (flags & ShaderBuilderFlagNormalMap)
				shader->SetUniformValue("u_NormalMap", i++);
			if (flags & ShaderBuilderFlagShadowMap)
				shader->SetUniformValue("u_DepthMap", 3);

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
				DD_ERR("{}", fragment);
				Application::s_Application->m_Window->FocusConsole(); // Make the error more noticeable
				return 0; // Get error from both shaders
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
				return 0;
			}
			glDeleteShader(vertexID);
			glDeleteShader(fragmentID);
			return result;
		}
	}
}