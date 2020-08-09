#include "basalt_private.hpp"

#include <algorithm>

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

    VkResult VkBasaltInstance::createDevice(VkPhysicalDevice             physDevice,
                                            const VkDeviceCreateInfo*    pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator,
                                            VkBasaltDevice**             basaltDevice,
                                            VkDevice*                    pDevice,
                                            PFN_vkGetDeviceProcAddr      gdpa) const
    {
        VkDeviceCreateInfo ourCreateInfo = *pCreateInfo;

        std::vector<const char*> exts(ourCreateInfo.ppEnabledExtensionNames,
                                      ourCreateInfo.ppEnabledExtensionNames + ourCreateInfo.enabledExtensionCount);

        auto addExt = [&exts](const char* newExt) {
            if (std::find(exts.begin(), exts.end(), newExt) == exts.end())
                exts.push_back(newExt);
        };

        addExt("VK_KHR_image_format_list");
        addExt("VK_KHR_swapchain_mutable_format");

        ourCreateInfo.ppEnabledExtensionNames = exts.data();
        ourCreateInfo.enabledExtensionCount   = exts.size();

        VkResult res = m_dispatch.CreateDevice(physDevice, &ourCreateInfo, pAllocator, pDevice);
        if (res != VK_SUCCESS)
            return res;

        *basaltDevice = new VkBasaltDevice(this, physDevice, *pDevice, gdpa);

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
