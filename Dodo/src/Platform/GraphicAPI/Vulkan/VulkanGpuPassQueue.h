#pragma once

#include "VulkanGpuPass.h"

#include <memory>
#include <vector>

namespace Dodo::Platform {

    /**
     * Manages a pool of in-flight one-shot GPU passes.
     * Submit() records a pass into a fresh command buffer and fires it asynchronously.
     * Poll() is called each frame to detect completion and invoke Finalize() on done passes.
     * WaitAll() blocks until all pending passes complete (safe to call at shutdown or init time).
     */
    class VulkanGpuPassQueue {
      public:
        explicit VulkanGpuPassQueue(VulkanGpuPassContext ctx);
        ~VulkanGpuPassQueue();

        VulkanGpuPassQueue(const VulkanGpuPassQueue&) = delete;
        VulkanGpuPassQueue& operator=(const VulkanGpuPassQueue&) = delete;

        void Submit(std::unique_ptr<VulkanGpuPass> pass);
        void Poll();
        void WaitAll();

      private:
        struct PendingPass {
            std::unique_ptr<VulkanGpuPass> pass;
            VkCommandBuffer cmd;
            VkFence fence;
        };

        void FinalizePending(PendingPass& p);

        VulkanGpuPassContext m_Ctx;
        std::vector<PendingPass> m_Pending;
    };

} // namespace Dodo::Platform
