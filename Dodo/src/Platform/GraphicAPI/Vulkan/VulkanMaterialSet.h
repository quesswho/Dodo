#pragma once

#include <volk.h>

namespace Dodo::Platform {

    class VulkanMaterialSet {
      public:
        bool IsAllocated() const { return m_Sets[0] != VK_NULL_HANDLE; }
        VkDescriptorSet Get(uint32_t frame) const { return m_Sets[frame]; }
        void MarkDirty() { m_Dirty = true; }
        bool IsDirty() const { return m_Dirty; }
        void ClearDirty() { m_Dirty = false; }
        void Assign(VkDescriptorSet a, VkDescriptorSet b) { m_Sets[0] = a; m_Sets[1] = b; }
        void Reset() { m_Sets[0] = VK_NULL_HANDLE; m_Sets[1] = VK_NULL_HANDLE; m_Dirty = true; }

      private:
        VkDescriptorSet m_Sets[2] = {};
        bool m_Dirty = true;
    };

} // namespace Dodo::Platform
