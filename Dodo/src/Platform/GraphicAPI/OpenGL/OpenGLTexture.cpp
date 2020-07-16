#include "pch.h"
#include "OpenGLTexture.h"

#include <glad/gl.h>
#include <stb_image.h>
#include "Core/Math/MathFunc.h"

namespace Dodo {

	namespace Platform {

		OpenGLTexture::OpenGLTexture(const char* path, uint index, const TextureProp& prop)
			: m_Index(index)
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);
			glBindTexture(GL_TEXTURE_2D, m_TextureID);

			switch (prop.m_WrapU)
			{
				case TextureWrapMode::WRAP_REPEAT:
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					break;
				case TextureWrapMode::WRAP_CLAMP_TO_BORDER:
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
					break;
				case TextureWrapMode::WRAP_CLAMP_TO_EDGE:
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					break;
				case TextureWrapMode::WRAP_MIRRORED_REPEAT:
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
					break;
			}

			switch (prop.m_WrapV)
			{
				case TextureWrapMode::WRAP_REPEAT:
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
					break;
				case TextureWrapMode::WRAP_CLAMP_TO_BORDER:
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
					break;
				case TextureWrapMode::WRAP_CLAMP_TO_EDGE:
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					break;
				case TextureWrapMode::WRAP_MIRRORED_REPEAT:
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
					break;
			}

			uint filter = static_cast<uint>(prop.m_Filter);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter < 4 ? GL_LINEAR : GL_NEAREST);
			if(filter > 2 && filter < 6)
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter & 1 ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);
			else
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter & 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST);

			if (prop.m_WrapU == TextureWrapMode::WRAP_CLAMP_TO_BORDER || prop.m_WrapV == TextureWrapMode::WRAP_CLAMP_TO_BORDER)
			{
				float borderColor[] = { 1.0f, 0.4f, 0.8f, 1.0f };
				glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
			}

			int  channels;
			stbi_set_flip_vertically_on_load(true);
			uchar* data = stbi_load(path, &m_Width, &m_Height, &channels, 0);
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

				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, internalFormat, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			else
			{
				DD_ERR("Could not load texture: {}", path);
			}
			stbi_image_free(data);
		}

		OpenGLTexture::~OpenGLTexture()
		{
			glDeleteTextures(1, &m_TextureID);
		}

		void OpenGLTexture::Bind() const
		{
			glActiveTexture(GL_TEXTURE0 + m_Index);
			glBindTexture(GL_TEXTURE_2D, m_TextureID);
		}
	}
}