#pragma once

#include "VulkanDescriptorSet.h"

namespace Dodo::Platform {

    /**
     * Owns one VulkanDescriptorSet per frame in flight (2) plus a per-frame
     * dirty mask and write-epoch. Re-writing a descriptor set already bound
     * to an in-flight command buffer is undefined, so callers should only
     * (re)write the set for a frame when both:
     *     IsDirtyForFrame(frame) is true, AND
     *     WasUpdatedThisEpoch(frame, currentEpoch) is false.
     * After flushing writes, call SetUpdatedEpoch + ClearDirtyForFrame.
     */
    class VulkanFrameBufferedDescriptorSet {
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
