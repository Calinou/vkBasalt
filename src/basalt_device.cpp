#include "basalt_private.hpp"

#include "generated/vulkan_dispatch_init.hpp"

namespace vkBasalt
{
    VkBasaltDevice::VkBasaltDevice(const VkBasaltInstance* basaltInstance,
                                   VkPhysicalDevice        _physDevice,
                                   VkDevice                _device,
                                   PFN_vkGetDeviceProcAddr gdpa) :
        instance(basaltInstance),
        device(_device), physDevice(_physDevice)
    {
        this->vk = instance->vk;
        initDeviceTable(gdpa, device, &this->vk);
    }

    VkBasaltDevice::~VkBasaltDevice()
    {
    }
} // namespace vkBasalt
