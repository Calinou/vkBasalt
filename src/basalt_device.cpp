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

    struct QueueFamilyInfo
    {
        uint32_t index;
        uint32_t count;
    };

    static QueueFamilyInfo findGeneralQueueFamily(const VkBasaltInstance* instance, VkPhysicalDevice physDevice)
    {
        constexpr VkQueueFlags wantedFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;

        uint32_t queueFamilyCount = 0;

        instance->vk().GetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);

        instance->vk().GetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount, queueFamilyProperties.data());

        uint32_t queueFamilyIndex = 0x70AD;
        uint32_t queueCount       = 0;

        VkQueueFlags currentFlags = VK_QUEUE_FLAG_BITS_MAX_ENUM;

        for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
        {
            auto& queueFamily = queueFamilyProperties[i];
            if (((queueFamily.queueFlags & wantedFlags) == wantedFlags) && queueFamily.queueFlags < currentFlags)
            {
                queueFamilyIndex = i;
                queueCount       = queueFamily.queueCount;
                currentFlags     = queueFamily.queueFlags;
            }
        }

        if (queueFamilyIndex == 0x70AD)
        {
            Logger::err("could not find general queue family");
        }

        return {queueFamilyIndex, queueCount};
    }

    void
    ensureGraphicsQueue(const VkBasaltInstance* instance, VkPhysicalDevice physDevice, LazyAllocator* allocator, VkDeviceCreateInfo* deviceCreateInfo)
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(deviceCreateInfo->pQueueCreateInfos,
                                                              deviceCreateInfo->pQueueCreateInfos + deviceCreateInfo->queueCreateInfoCount);

        QueueFamilyInfo generalFamily = findGeneralQueueFamily(instance, physDevice);

        auto it = std::find_if(
            queueCreateInfos.begin(), queueCreateInfos.end(), [&generalFamily](const auto& q) { return q.queueFamilyIndex == generalFamily.index; });

        if (it == queueCreateInfos.end())
        {
            // we need one general queue for us
            float* prio = allocator->alloc<float>();
            *prio       = 1.0f;

            queueCreateInfos.push_back({
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext            = nullptr,
                .flags            = 0,
                .queueFamilyIndex = generalFamily.index,
                .queueCount       = 1,
                .pQueuePriorities = prio,
            });
        }
        else
        {
            if (it->queueCount < generalFamily.count)
            {
                // create one more queue if possible to avoid extern sync problems
                it->queueCount++;
                float* prios = allocator->alloc<float>(it->queueCount);

                std::memcpy(prios, it->pQueuePriorities, (it->queueCount - 1) * sizeof(float));
                prios[it->queueCount - 1] = 1.0f;
            }
        }

        VkDeviceQueueCreateInfo* queueMem = allocator->alloc<VkDeviceQueueCreateInfo>(queueCreateInfos.size());
        std::memcpy(queueMem, queueCreateInfos.data(), queueCreateInfos.size() * sizeof(VkDeviceQueueCreateInfo));

        deviceCreateInfo->pQueueCreateInfos    = queueMem;
        deviceCreateInfo->queueCreateInfoCount = queueCreateInfos.size();
    }

    VkBasaltDevice::VkBasaltDevice(const VkBasaltInstance* basaltInstance, const VkBasaltDeviceCreateInfo* createInfo) :
        m_instance(basaltInstance), m_device(*createInfo->pDevice), m_physDevice(createInfo->physDevice)
    {
        m_dispatch = m_instance->vk();
        initDeviceTable(createInfo->gdpa, m_device, &m_dispatch);

        const VkDeviceQueueCreateInfo* queueInfos     = createInfo->pCreateInfo->pQueueCreateInfos;
        uint32_t                       queueInfoCount = createInfo->pCreateInfo->queueCreateInfoCount;

        for (auto queueInfo = queueInfos; queueInfo < queueInfos + queueInfoCount; queueInfo++)
        {
            for (uint32_t index = 0; index < queueInfo->queueCount; index++)
            {
                VkQueue queue;
                vk().GetDeviceQueue(m_device, queueInfo->queueFamilyIndex, index, &queue);

                m_familyIndices[queue] = queueInfo->queueFamilyIndex;
            }
        }

        // Get the last general queue
        QueueFamilyInfo generalFamily = findGeneralQueueFamily(m_instance, m_physDevice);

        auto it = std::find_if(
            queueInfos, queueInfos + queueInfoCount, [&generalFamily](const auto q) { return q.queueFamilyIndex == generalFamily.index; });
        assert(it != queueInfos + queueInfoCount);

        VkQueue queue;
        vk().GetDeviceQueue(m_device, it->queueFamilyIndex, it->queueCount - 1, &queue);
        queue->key = m_device->key;

        m_generalQueue = std::make_unique<VkBasaltQueue>(this, queue, it->queueFamilyIndex);
    }

    VkBasaltDevice::~VkBasaltDevice()
    {
    }
} // namespace vkBasalt
