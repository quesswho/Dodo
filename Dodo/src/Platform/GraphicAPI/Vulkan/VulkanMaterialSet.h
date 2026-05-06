#pragma once

#include "VulkanDescriptorSet.h"

namespace Dodo::Platform {

    class VulkanMaterialSet {
      public:
        bool IsAllocated() const { return m_Sets[0].IsValid(); }

        VulkanDescriptorSet& Get(uint32_t frame) { return m_Sets[frame]; }
        const VulkanDescriptorSet& Get(uint32_t frame) const { return m_Sets[frame]; }

        void MarkDirty() { m_DirtyMask = 0x3u; }
        bool IsDirtyForFrame(uint32_t frame) const { return (m_DirtyMask >> frame) & 1u; }
        void ClearDirtyForFrame(uint32_t frame) { m_DirtyMask &= ~(1u << frame); }
        bool WasUpdatedThisEpoch(uint32_t frame, uint32_t epoch) const { return m_LastEpoch[frame] == epoch; }
        void SetUpdatedEpoch(uint32_t frame, uint32_t epoch) { m_LastEpoch[frame] = epoch; }

        void Reset()
        {
            m_Sets[0].Invalidate();
            m_Sets[1].Invalidate();
            m_DirtyMask = 0x3u;
            m_LastEpoch[0] = m_LastEpoch[1] = UINT32_MAX;
        }

      private:
        VulkanDescriptorSet m_Sets[2];
        uint32_t m_DirtyMask = 0x3u;
        uint32_t m_LastEpoch[2] = {UINT32_MAX, UINT32_MAX};
    };

} // namespace Dodo::Platform
