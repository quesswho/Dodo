#include "VkCubeMap.h"
#include "pch.h"

#include <glad/gl.h>
#include <stb_image.h>

namespace Dodo { namespace Platform {

    VkCubeMapTexture::VkCubeMapTexture(std::vector<std::string> paths, uint index, const TextureSettings& prop)
        : m_Index(index)
    {
        
    }

    VkCubeMapTexture::~VkCubeMapTexture()
    {
       
    }

    void VkCubeMapTexture::Bind() const
    {
        
    }
}} // namespace Dodo::Platform