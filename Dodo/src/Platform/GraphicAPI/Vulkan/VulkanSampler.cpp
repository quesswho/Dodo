#include "VulkanSampler.h"

#include "Core/Utilities/Logger.h"

namespace Dodo::Platform {

    static void DecodeFilter(SamplerFilter filter, VkFilter& minFilter, VkFilter& magFilter,
                             VkSamplerMipmapMode& mipMode)
    {
        switch (filter) {
        case SamplerFilter::MIN_MAG_LINEAR:
            minFilter = VK_FILTER_LINEAR;
            magFilter = VK_FILTER_LINEAR;
            mipMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case SamplerFilter::MIN_MAG_NEAREST:
            minFilter = VK_FILTER_NEAREST;
            magFilter = VK_FILTER_NEAREST;
            mipMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case SamplerFilter::MIN_MAG_MIP_LINEAR:
            minFilter = VK_FILTER_LINEAR;
            magFilter = VK_FILTER_LINEAR;
            mipMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        case SamplerFilter::MIN_LINEAR_MAG_MIP_NEAREST:
            minFilter = VK_FILTER_LINEAR;
            magFilter = VK_FILTER_NEAREST;
            mipMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case SamplerFilter::MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
            minFilter = VK_FILTER_LINEAR;
            magFilter = VK_FILTER_NEAREST;
            mipMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        case SamplerFilter::MIN_MAG_LINEAR_MIP_NEAREST:
            minFilter = VK_FILTER_LINEAR;
            magFilter = VK_FILTER_LINEAR;
            mipMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case SamplerFilter::FILTER_MIN_MAG_MIP_NEAREST:
            minFilter = VK_FILTER_NEAREST;
            magFilter = VK_FILTER_NEAREST;
            mipMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case SamplerFilter::FILTER_MIN_MAG_NEAREST_MIP_LINEAR:
            minFilter = VK_FILTER_NEAREST;
            magFilter = VK_FILTER_NEAREST;
            mipMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        case SamplerFilter::FILTER_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
            minFilter = VK_FILTER_NEAREST;
            magFilter = VK_FILTER_LINEAR;
            mipMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case SamplerFilter::FILTER_MIN_NEAREST_MAG_MIP_LINEAR:
            minFilter = VK_FILTER_NEAREST;
            magFilter = VK_FILTER_LINEAR;
            mipMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        default:
            minFilter = VK_FILTER_LINEAR;
            magFilter = VK_FILTER_LINEAR;
            mipMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        }
    }

    static VkSamplerAddressMode ToVkWrap(SamplerWrapMode mode)
    {
        switch (mode) {
        case SamplerWrapMode::WRAP_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerWrapMode::WRAP_CLAMP_TO_BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case SamplerWrapMode::WRAP_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SamplerWrapMode::WRAP_MIRRORED_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        default:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }

    // Maps a float border color to the nearest standard VkBorderColor.
    // TODO: VK_EXT_custom_border_color would allow arbitrary colors but requires
    // an additional extension + device feature
    static VkBorderColor ToVkBorderColor(const float c[4])
    {
        bool opaque = c[3] >= 0.5f;
        bool white = (c[0] + c[1] + c[2]) >= 1.5f;
        if (opaque) return white ? VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE : VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    }

    VulkanSampler::VulkanSampler(const SamplerProperties& prop, VkDevice device) : m_Device(device)
    {
        VkFilter minFilter, magFilter;
        VkSamplerMipmapMode mipMode;
        DecodeFilter(prop.m_Filter, minFilter, magFilter, mipMode);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.minFilter = minFilter;
        samplerInfo.magFilter = magFilter;
        samplerInfo.mipmapMode = mipMode;
        samplerInfo.addressModeU = ToVkWrap(prop.m_WrapU);
        samplerInfo.addressModeV = ToVkWrap(prop.m_WrapV);
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
        samplerInfo.borderColor = ToVkBorderColor(prop.m_BorderColor);
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        if (vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS)
            DD_ERR("VulkanSampler: failed to create sampler!");
    }

    VulkanSampler::~VulkanSampler()
    {
        vkDestroySampler(m_Device, m_Sampler, nullptr);
    }
} // namespace Dodo::Platform
