#include "logical_swapchain.hpp"

namespace vkBasalt
{
    void LogicalSwapchain::destroy()
    {
        if (imageCount > 0)
        {
            effects.clear();
            defaultTransfer.reset();

            pLogicalDevice->vkd.FreeCommandBuffers(
                pLogicalDevice->device, pLogicalDevice->commandPool, commandBuffersEffect.size(), commandBuffersEffect.data());
            pLogicalDevice->vkd.FreeCommandBuffers(
                pLogicalDevice->device, pLogicalDevice->commandPool, commandBuffersNoEffect.size(), commandBuffersNoEffect.data());
            Logger::debug("after free commandbuffer");

            for (uint32_t i = 0; i < fakeImages.size(); i++)
            {
                fakeImages[i].destroy(pLogicalDevice);
            }

            for (unsigned int i = 0; i < imageCount; i++)
            {
                pLogicalDevice->vkd.DestroySemaphore(pLogicalDevice->device, semaphores[i], nullptr);
            }
            Logger::debug("after DestroySemaphore");
        }
    }
} // namespace vkBasalt
