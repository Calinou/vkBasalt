#include "basalt_private.hpp"

#include <utility>

namespace vkBasalt
{
    VkBasaltQueue::VkBasaltQueue(VkBasaltDevice* basaltDevice, VkQueue queue, uint32_t queueFamily) :
        m_device(basaltDevice), m_queue(queue), m_family(queueFamily)
    {
        VkCommandPoolCreateInfo commandPoolCreateInfo = {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext            = nullptr,
            .flags            = 0,
            .queueFamilyIndex = m_family,
        };

        VkResult res = m_device->vk().CreateCommandPool(m_device->get(), &commandPoolCreateInfo, nullptr, &m_cmdPool);
        ASSERT_VULKAN(res);
    }

    VkBasaltQueue::~VkBasaltQueue()
    {
        m_device->vk().DestroyCommandPool(m_device->get(), m_cmdPool, nullptr);
    }

    VkBasaltQueue::VkBasaltQueue(VkBasaltQueue&& other) :
        m_device(other.m_device), m_queue(other.m_queue), m_cmdPool(std::exchange(other.m_cmdPool, VkCommandPool(VK_NULL_HANDLE))),
        m_family(other.m_family)
    {
    }

    VkBasaltQueue& VkBasaltQueue::operator=(VkBasaltQueue&& other)
    {
        m_family = other.m_family;
        m_queue  = other.m_queue;
        std::swap(m_device, other.m_device);
        std::swap(m_cmdPool, other.m_cmdPool);

        return *this;
    }
} // namespace vkBasalt
