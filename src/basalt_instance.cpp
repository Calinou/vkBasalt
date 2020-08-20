#include "basalt_private.hpp"

#include <algorithm>

#include <lazy_allocator.hpp>

#include "generated/vulkan_dispatch_init.hpp"

namespace vkBasalt
{
    VkResult VkBasaltInstance::createInstance(const VkInstanceCreateInfo*  pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator,
                                              PFN_vkGetInstanceProcAddr    gipa,
                                              VkBasaltInstance**           basaltInstance,
                                              VkInstance*                  pInstance)
    {
        VkInstanceCreateInfo ourCreateInfo = *pCreateInfo;

        VkApplicationInfo appInfo;

        if (ourCreateInfo.pApplicationInfo)
        {
            appInfo = *ourCreateInfo.pApplicationInfo;
            if (appInfo.apiVersion < VK_API_VERSION_1_1)
                appInfo.apiVersion = VK_API_VERSION_1_1;
        }
        else
        {
            appInfo = {
                .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pNext              = nullptr,
                .pApplicationName   = nullptr,
                .applicationVersion = 0,
                .pEngineName        = nullptr,
                .engineVersion      = 0,
                .apiVersion         = VK_API_VERSION_1_1,
            };
        }
        ourCreateInfo.pApplicationInfo = &appInfo;

        PFN_vkCreateInstance createFunc = (PFN_vkCreateInstance) gipa(VK_NULL_HANDLE, "vkCreateInstance");

        VkResult res = createFunc(&ourCreateInfo, pAllocator, pInstance);
        if (res != VK_SUCCESS)
            return res;

        *basaltInstance = new VkBasaltInstance(*pInstance, gipa);

        return VK_SUCCESS;
    }

    void VkBasaltInstance::destroyInstance(VkBasaltInstance* basaltInstance, const VkAllocationCallbacks* pAllocator)
    {
        VkInstance instance = basaltInstance->m_instance;

        PFN_vkDestroyInstance destroyFunc = basaltInstance->m_dispatch.DestroyInstance;

        delete basaltInstance;

        destroyFunc(instance, pAllocator);
    }

    VkBasaltInstance::VkBasaltInstance(VkInstance instance, PFN_vkGetInstanceProcAddr gipa) : m_instance(instance)
    {
        initInstanceTable(gipa, instance, &m_dispatch);
    }

    VkBasaltInstance::~VkBasaltInstance()
    {
    }

    VkResult VkBasaltInstance::createDevice(VkBasaltDeviceCreateInfo* createInfo, VkBasaltDevice** basaltDevice) const
    {
        VkBasaltDeviceCreateInfo ourCreateInfo    = *createInfo;
        VkDeviceCreateInfo       deviceCreateInfo = *createInfo->pCreateInfo;
        ourCreateInfo.pCreateInfo                 = &deviceCreateInfo;

        LazyAllocator allocator;

        activateDeviceExtensions(this, ourCreateInfo.physDevice, &allocator, &deviceCreateInfo);

        VkResult res = m_dispatch.CreateDevice(ourCreateInfo.physDevice, &deviceCreateInfo, ourCreateInfo.pAllocator, ourCreateInfo.pDevice);
        if (res != VK_SUCCESS)
            return res;

        *basaltDevice = new VkBasaltDevice(this, &ourCreateInfo);

        return VK_SUCCESS;
    }

    void VkBasaltInstance::destroyDevice(VkBasaltDevice* basaltDevice, const VkAllocationCallbacks* pAllocator) const
    {
        VkDevice device = basaltDevice->get();

        PFN_vkDestroyDevice destroyFunc = basaltDevice->vk().DestroyDevice;

        delete basaltDevice;

        destroyFunc(device, pAllocator);
    }
} // namespace vkBasalt
