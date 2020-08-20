#include "basalt_private.hpp"

#include <cstring>

#include "lazy_allocator.hpp"

#include "generated/vulkan_dispatch_init.hpp"

namespace vkBasalt
{
    void activateDeviceExtensions(const VkBasaltInstance* instance,
                                  VkPhysicalDevice        physDevice,
                                  LazyAllocator*          allocator,
                                  VkDeviceCreateInfo*     deviceCreateInfo)
    {
        std::vector<const char*> exts(deviceCreateInfo->ppEnabledExtensionNames,
                                      deviceCreateInfo->ppEnabledExtensionNames + deviceCreateInfo->enabledExtensionCount);

        uint32_t availableCount;
        instance->vk().EnumerateDeviceExtensionProperties(physDevice, nullptr, &availableCount, nullptr);

        std::vector<VkExtensionProperties> availableExts(availableCount);
        instance->vk().EnumerateDeviceExtensionProperties(physDevice, nullptr, &availableCount, availableExts.data());

        auto addExt = [&exts, &availableExts](const char* newExt) {
            if (std::find_if(availableExts.begin(), availableExts.end(), [&newExt](const auto& e) { return !std::strcmp(e.extensionName, newExt); })
                == availableExts.end())
            {
                return;
            }

            if (std::find(exts.begin(), exts.end(), newExt) == exts.end())
                exts.push_back(newExt);
        };

        addExt("VK_KHR_image_format_list");
        addExt("VK_KHR_swapchain_mutable_format");

        char** extsMem = allocator->alloc<char*>(exts.size());
        std::memcpy(extsMem, exts.data(), exts.size() * sizeof(char*));

        deviceCreateInfo->ppEnabledExtensionNames = extsMem;
        deviceCreateInfo->enabledExtensionCount   = exts.size();
    }

    VkBasaltDevice::VkBasaltDevice(const VkBasaltInstance* basaltInstance, const VkBasaltDeviceCreateInfo* createInfo) :
        m_instance(basaltInstance), m_device(*createInfo->pDevice), m_physDevice(createInfo->physDevice)
    {
        m_dispatch = m_instance->vk();
        initDeviceTable(createInfo->gdpa, m_device, &m_dispatch);
    }

    VkBasaltDevice::~VkBasaltDevice()
    {
    }
} // namespace vkBasalt
