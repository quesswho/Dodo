#include "pch.h"
#include "OpenGLTexture.h"

#include <glad/gl.h>
#include <stb_image.h>
#include "Core/Math/MathFunc.h"

namespace Dodo {

	namespace Platform {

		OpenGLTexture::OpenGLTexture(const char* path, uint index, const TextureSettings& settings)
			: m_Index(index), m_TextureID(0)
		{
			int width, height, channels;
			stbi_set_flip_vertically_on_load(true);
			uchar* data = stbi_load(path, &width, &height, &channels, 0);
			if (data)
			{
				TextureProperties props((uint)width, (uint)height);
				switch (channels)
				{
					case 3:
						props.m_Format = TextureFormat::FORMAT_RGB;
						break;
					case 4:
						props.m_Format = TextureFormat::FORMAT_RGBA;
						break;
					default:
						DD_ERR("File format is not supported! {}", channels);
				}
				m_TextureProperties = props;
				Init(data, settings);
			}
			else
			{
				DD_ERR("Could not load texture: {}", path);
			}
			stbi_image_free(data);
		}

		OpenGLTexture::OpenGLTexture(uchar* data, TextureProperties prop, uint index, const TextureSettings& settings)
			: m_Index(index), m_TextureProperties(prop)
		{
			Init(data, settings);
		}

		void OpenGLTexture::Init(uchar* data, const TextureSettings& settings)
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);
			glBindTexture(GL_TEXTURE_2D, m_TextureID);

			switch (settings.m_WrapU)
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

			switch (settings.m_WrapV)
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

			uint filter = static_cast<uint>(settings.m_Filter);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter < 4 ? GL_LINEAR : GL_NEAREST);

			if (filter > 2 && filter < 6)
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			else
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			if (settings.m_WrapU == TextureWrapMode::WRAP_CLAMP_TO_BORDER || settings.m_WrapV == TextureWrapMode::WRAP_CLAMP_TO_BORDER)
			{
				float borderColor[] = { 1.0f, 0.4f, 0.8f, 0.09f };
				glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
			}

			uint format;
			switch (m_TextureProperties.m_Format)
			{
				case TextureFormat::FORMAT_RGB:
					format = GL_RGB;
					break;
				case TextureFormat::FORMAT_RGBA:
					format = GL_RGBA;
					break;
				default:
					format = GL_RGB;
			}
			glTexImage2D(GL_TEXTURE_2D, 0, format, m_TextureProperties.m_Width, m_TextureProperties.m_Height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
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