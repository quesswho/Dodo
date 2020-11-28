#include "pch.h"
#include "OpenGLCubeMap.h"

#include <glad/gl.h>
#include <stb_image.h>

namespace Dodo {

	namespace Platform {

		OpenGLCubeMapTexture::OpenGLCubeMapTexture(std::vector<std::string> paths, uint index, const TextureSettings& prop)
			: m_Index(index)
		{
			glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_TextureID);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);

			switch (prop.m_WrapU)
			{
				case TextureWrapMode::WRAP_REPEAT:
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
					break;
				case TextureWrapMode::WRAP_CLAMP_TO_BORDER:
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
					break;
				case TextureWrapMode::WRAP_CLAMP_TO_EDGE:
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					break;
				case TextureWrapMode::WRAP_MIRRORED_REPEAT:
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
					break;
			}

			switch (prop.m_WrapV)
			{
				case TextureWrapMode::WRAP_REPEAT:
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
					break;
				case TextureWrapMode::WRAP_CLAMP_TO_BORDER:
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
					break;
				case TextureWrapMode::WRAP_CLAMP_TO_EDGE:
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					break;
				case TextureWrapMode::WRAP_MIRRORED_REPEAT:
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
					break;
			}

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			uint filter = static_cast<uint>(prop.m_Filter);

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, filter < 4 ? GL_LINEAR : GL_NEAREST);
			if (filter > 2 && filter < 6)
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			else
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			if (prop.m_WrapU == TextureWrapMode::WRAP_CLAMP_TO_BORDER || prop.m_WrapV == TextureWrapMode::WRAP_CLAMP_TO_BORDER)
			{
				float borderColor[] = { 1.0f, 0.4f, 0.8f, 1.0f };
				glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, borderColor);
			}

			for (int i = 0; i < paths.size(); i++)
			{
				int channels, width, height;
				uchar* data = stbi_load(paths[i].c_str(), &width, &height, &channels, 0);
				if (data)
				{
					int internalFormat = 0;
					switch (channels)
					{
						case 3:
							internalFormat = GL_RGB;
							break;
						case 4:
							internalFormat = GL_RGBA;
							break;
						default:
							DD_ERR("File format is not supported! {}", channels);
					}

					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, internalFormat, GL_UNSIGNED_BYTE, data);
				}
				else
				{
					DD_ERR("Could not load texture: {}", paths[i]);
				}
				stbi_image_free(data);
			}
		}

		OpenGLCubeMapTexture::~OpenGLCubeMapTexture()
		{
			glDeleteTextures(1, &m_TextureID);
		}

		void OpenGLCubeMapTexture::Bind() const
		{
			glActiveTexture(GL_TEXTURE0 + m_Index);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);
		}
	}
}