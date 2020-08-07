#include "basalt_private.hpp"

#include "generated/vulkan_dispatch_init.hpp"

namespace vkBasalt
{
    VkResult VkBasaltInstance::createInstance(const VkInstanceCreateInfo*  pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator,
                                              PFN_vkGetInstanceProcAddr    gipa,
                                              VkBasaltInstance**           basaltInstance)
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

        VkInstance instance;

        VkResult res = createFunc(&ourCreateInfo, pAllocator, &instance);
        if (res != VK_SUCCESS)
            return res;

        *basaltInstance = new VkBasaltInstance(instance, gipa);

        return VK_SUCCESS;
    }

    void VkBasaltInstance::destroyInstance(VkBasaltInstance* basaltInstance, const VkAllocationCallbacks* pAllocator)
    {
        VkInstance instance = basaltInstance->instance;

        PFN_vkDestroyInstance destroyFunc = basaltInstance->vk.DestroyInstance;

        delete basaltInstance;

        destroyFunc(instance, pAllocator);
    }

    VkBasaltInstance::VkBasaltInstance(VkInstance _instance, PFN_vkGetInstanceProcAddr gipa) : instance(_instance)
    {
        initInstanceTable(gipa, instance, &this->vk);
    }

    VkBasaltInstance::~VkBasaltInstance()
    {
    }

    VkResult VkBasaltInstance::createDevice(VkPhysicalDevice             physDevice,
                                            const VkDeviceCreateInfo*    pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator,
                                            VkBasaltDevice**             basaltDevice) const
    {
        VkDeviceCreateInfo ourCreateInfo = *pCreateInfo;

        VkDevice device;

        VkResult res = vk.CreateDevice(physDevice, &ourCreateInfo, pAllocator, &device);
        if (res != VK_SUCCESS)
            return res;

        *basaltDevice = new VkBasaltDevice(this, physDevice, device, vk.GetDeviceProcAddr);

        return VK_SUCCESS;
    }

    void VkBasaltInstance::destroyDevice(VkBasaltDevice* basaltDevice, const VkAllocationCallbacks* pAllocator) const
    {
        VkDevice device = basaltDevice->device;

        delete basaltDevice;

        vk.DestroyDevice(device, pAllocator);
    }
} // namespace vkBasalt
