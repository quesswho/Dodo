#include "VulkanGpuPassQueue.h"

#include <vk_mem_alloc.h>

namespace Dodo::Platform {

    VulkanGpuPassQueue::VulkanGpuPassQueue(VulkanGpuPassContext ctx)
        : m_Ctx(ctx)
    {}

    VulkanGpuPassQueue::~VulkanGpuPassQueue()
    {
        WaitAll();
    }

    void VulkanGpuPassQueue::Submit(std::unique_ptr<VulkanGpuPass> pass)
    {
        VkCommandBufferAllocateInfo cbAllocInfo{};
        cbAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbAllocInfo.commandPool        = m_Ctx.commandPool;
        cbAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cbAllocInfo.commandBufferCount = 1;
        VkCommandBuffer cmd = VK_NULL_HANDLE;
        vkAllocateCommandBuffers(m_Ctx.device, &cbAllocInfo, &cmd);

        VkCommandBufferBeginInfo cbBegin{};
        cbBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cbBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &cbBegin);

        pass->Record(cmd, m_Ctx);

        vkEndCommandBuffer(cmd);

        VkFenceCreateInfo fenceCI{};
        fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence = VK_NULL_HANDLE;
        vkCreateFence(m_Ctx.device, &fenceCI, nullptr, &fence);

        VkSubmitInfo si{};
        si.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        si.commandBufferCount = 1;
        si.pCommandBuffers    = &cmd;
        vkQueueSubmit(m_Ctx.graphicsQueue, 1, &si, fence);

        m_Pending.push_back({std::move(pass), cmd, fence});
    }

    void VulkanGpuPassQueue::Poll()
    {
        for (auto it = m_Pending.begin(); it != m_Pending.end();) {
            if (vkGetFenceStatus(m_Ctx.device, it->fence) == VK_SUCCESS) {
                FinalizePending(*it);
                it = m_Pending.erase(it);
            } else {
                ++it;
            }
        }
    }

    void VulkanGpuPassQueue::WaitAll()
    {
        for (auto& pending : m_Pending) {
            vkWaitForFences(m_Ctx.device, 1, &pending.fence, VK_TRUE, UINT64_MAX);
            FinalizePending(pending);
        }
        m_Pending.clear();
    }

    void VulkanGpuPassQueue::FinalizePending(PendingPass& p)
    {
        p.pass->Finalize();
        vkFreeCommandBuffers(m_Ctx.device, m_Ctx.commandPool, 1, &p.cmd);
        vkDestroyFence(m_Ctx.device, p.fence, nullptr);
    }

} // namespace Dodo::Platform
