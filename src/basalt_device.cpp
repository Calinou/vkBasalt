#include "basalt_private.hpp"

#include "generated/vulkan_dispatch_init.hpp"

namespace vkBasalt
{
    VkBasaltDevice::VkBasaltDevice(const VkBasaltInstance* basaltInstance, VkPhysicalDevice physDevice, VkDevice device) :
        m_instance(basaltInstance), m_device(device), m_physDevice(physDevice)
    {
        this->vk = instance->vk;
        initDeviceTable(gdpa, device, &this->vk);
    }

    VkBasaltDevice::~VkBasaltDevice()
    {
    }
} // namespace vkBasalt
