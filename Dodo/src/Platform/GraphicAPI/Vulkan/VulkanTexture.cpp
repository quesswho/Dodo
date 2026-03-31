#include "VulkanTexture.h"
#include "pch.h"

namespace Dodo::Platform {

    VulkanTexture::VulkanTexture(uchar* data, const TextureProperties& prop) : m_TextureProperties(prop), m_TextureID(0)
    {
        Init(data);
    }

    void VulkanTexture::Init(uchar* data) {}

    VulkanTexture::~VulkanTexture() {}
} // namespace Dodo::Platform