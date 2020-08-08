#include "basalt_private.hpp"

#include "generated/vulkan_dispatch_init.hpp"

namespace vkBasalt
{
    VkBasaltDevice::VkBasaltDevice(const VkBasaltInstance* basaltInstance,
                                   VkPhysicalDevice        physDevice,
                                   VkDevice                device,
                                   PFN_vkGetDeviceProcAddr gdpa) :
        m_instance(basaltInstance),
        m_device(device), m_physDevice(physDevice)
    {
        m_dispatch = basaltInstance->vk();
        initDeviceTable(gdpa, device, &m_dispatch);
    }

    VkBasaltDevice::~VkBasaltDevice()
    {
    }
} // namespace vkBasalt
