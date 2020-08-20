#include <mutex>
#include <cstring>

#include "basalt_private.hpp"

#include "vulkan_pnext.hpp"

#define LAYER_NAME "VK_LAYER_VKBASALT_post_processing"

namespace vkBasalt
{
    // Every entry point needs to lock this
    static std::mutex g_lock;

    static inline std::scoped_lock<std::mutex> lockGlobally()
    {
        return std::scoped_lock<std::mutex>(g_lock);
    }

    static std::unordered_map<InstanceKey, VkBasaltInstance*> g_instanceMap;
    static std::unordered_map<DeviceKey, VkBasaltDevice*>     g_deviceMap;

    static inline VkBasaltInstance* getBasaltInstance(InstanceKey key)
    {
        return g_instanceMap[key];
    }

    static inline VkBasaltDevice* getBasaltDevice(DeviceKey key)
    {
        return g_deviceMap[key];
    }

    VkResult VKAPI_CALL vkBasalt_CreateInstance(const VkInstanceCreateInfo*  pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator,
                                                VkInstance*                  pInstance)
    {
        Logger::trace("vkCreateInstance");
        auto lock = lockGlobally();

        auto layerCreateInfo = pNextSearch<VkLayerInstanceCreateInfo>(pCreateInfo->pNext, [](auto x) { return x->function == VK_LAYER_LINK_INFO; });

        if (layerCreateInfo == nullptr)
            return VK_ERROR_INITIALIZATION_FAILED;

        PFN_vkGetInstanceProcAddr gipa = layerCreateInfo->u.pLayerInfo->pfnNextGetInstanceProcAddr;
        // move chain on for next layer
        layerCreateInfo->u.pLayerInfo = layerCreateInfo->u.pLayerInfo->pNext;

        VkBasaltInstance* basaltInstance;

        VkResult ret = VkBasaltInstance::createInstance(pCreateInfo, pAllocator, gipa, &basaltInstance, pInstance);
        if (ret != VK_SUCCESS)
            return ret;

        g_instanceMap[(*pInstance)->key] = basaltInstance;

        return VK_SUCCESS;
    }

    void VKAPI_CALL vkBasalt_DestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator)
    {
        Logger::trace("vkDestroyInstance");
        auto lock = lockGlobally();

        auto basaltInstance = getBasaltInstance(instance->key);

        g_instanceMap.erase(instance->key);

        VkBasaltInstance::destroyInstance(basaltInstance, pAllocator);
    }

    VkResult VKAPI_CALL vkBasalt_CreateDevice(VkPhysicalDevice             physicalDevice,
                                              const VkDeviceCreateInfo*    pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator,
                                              VkDevice*                    pDevice)
    {
        Logger::trace("vkCreateDevice");
        auto lock = lockGlobally();

        auto basaltInstance = getBasaltInstance(physicalDevice->key);

        auto layerCreateInfo = pNextSearch<VkLayerDeviceCreateInfo>(pCreateInfo->pNext, [](auto x) { return x->function == VK_LAYER_LINK_INFO; });

        if (layerCreateInfo == nullptr)
            return VK_ERROR_INITIALIZATION_FAILED;

        PFN_vkGetDeviceProcAddr gdpa = layerCreateInfo->u.pLayerInfo->pfnNextGetDeviceProcAddr;

        // move chain on for next layer
        layerCreateInfo->u.pLayerInfo = layerCreateInfo->u.pLayerInfo->pNext;

        VkBasaltDevice* basaltDevice;

        VkBasaltDeviceCreateInfo basaltCreateInfo = {
            .physDevice  = physicalDevice,
            .pCreateInfo = pCreateInfo,
            .pAllocator  = pAllocator,
            .gdpa        = gdpa,
            .pDevice     = pDevice,
        };

        VkResult ret = basaltInstance->createDevice(&basaltCreateInfo, &basaltDevice);
        if (ret != VK_SUCCESS)
            return ret;

        g_deviceMap[(*pDevice)->key] = basaltDevice;

        return VK_SUCCESS;
    }

    void VKAPI_CALL vkBasalt_DestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
    {
        Logger::trace("vkDestroyDevice");
        auto lock = lockGlobally();

        auto basaltDevice = getBasaltDevice(device->key);

        g_deviceMap.erase(device->key);

        basaltDevice->instance()->destroyDevice(basaltDevice, pAllocator);
    }

    VkResult VKAPI_CALL vkBasalt_EnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties)
    {
        if (pPropertyCount)
            *pPropertyCount = 1;

        if (pProperties)
        {
            std::strcpy(pProperties->layerName, LAYER_NAME);
            std::strcpy(pProperties->description, "test");
            pProperties->implementationVersion = 1;
            pProperties->specVersion           = VK_MAKE_VERSION(1, 2, 0);
        }

        return VK_SUCCESS;
    }

    VkResult VKAPI_CALL vkBasalt_EnumerateDeviceLayerProperties(VkPhysicalDevice   physicalDevice,
                                                                uint32_t*          pPropertyCount,
                                                                VkLayerProperties* pProperties)
    {
        return vkBasalt_EnumerateInstanceLayerProperties(pPropertyCount, pProperties);
    }

    VkResult VKAPI_CALL vkBasalt_EnumerateInstanceExtensionProperties(const char*            pLayerName,
                                                                      uint32_t*              pPropertyCount,
                                                                      VkExtensionProperties* pProperties)
    {
        if (pLayerName == NULL || std::strcmp(pLayerName, LAYER_NAME))
        {
            return VK_ERROR_LAYER_NOT_PRESENT;
        }

        // don't expose any extensions
        if (pPropertyCount)
        {
            *pPropertyCount = 0;
        }
        return VK_SUCCESS;
    }

    VkResult VKAPI_CALL vkBasalt_EnumerateDeviceExtensionProperties(VkPhysicalDevice       physicalDevice,
                                                                    const char*            pLayerName,
                                                                    uint32_t*              pPropertyCount,
                                                                    VkExtensionProperties* pProperties)
    {
        // pass through any queries that aren't to us
        if (pLayerName == NULL || std::strcmp(pLayerName, LAYER_NAME))
        {
            if (physicalDevice == VK_NULL_HANDLE)
            {
                return VK_SUCCESS;
            }

            auto lock = lockGlobally();

            auto basaltInstance = getBasaltInstance(physicalDevice->key);
            return basaltInstance->vk().EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
        }

        // don't expose any extensions
        if (pPropertyCount)
        {
            *pPropertyCount = 0;
        }
        return VK_SUCCESS;
    }
} // namespace vkBasalt

extern "C"
{
    VK_LAYER_EXPORT PFN_vkVoidFunction VKAPI_CALL vkBasalt_GetDeviceProcAddr(VkDevice device, const char* pName);
    VK_LAYER_EXPORT PFN_vkVoidFunction VKAPI_CALL vkBasalt_GetInstanceProcAddr(VkInstance instance, const char* pName);

#define GETPROCADDR(func)                \
    if (!std::strcmp(pName, "vk" #func)) \
        return (PFN_vkVoidFunction) &vkBasalt_##func;

    static PFN_vkVoidFunction interceptCalls(const char* pName)
    {
        using namespace vkBasalt;
        /* instance chain functions we intercept */
        GETPROCADDR(GetInstanceProcAddr);
        GETPROCADDR(EnumerateInstanceLayerProperties);
        GETPROCADDR(EnumerateInstanceExtensionProperties);
        GETPROCADDR(CreateInstance);
        GETPROCADDR(DestroyInstance);

        /* device chain functions we intercept*/
        GETPROCADDR(GetDeviceProcAddr);
        GETPROCADDR(EnumerateDeviceLayerProperties);
        GETPROCADDR(EnumerateDeviceExtensionProperties);
        GETPROCADDR(CreateDevice);
        GETPROCADDR(DestroyDevice);

        return nullptr;
    }

    VK_LAYER_EXPORT PFN_vkVoidFunction VKAPI_CALL vkBasalt_GetDeviceProcAddr(VkDevice device, const char* pName)
    {
        using namespace vkBasalt;

        auto lock = lockGlobally();

        auto layerFunc = interceptCalls(pName);
        if (layerFunc)
            return layerFunc;

        auto basaltDevice = getBasaltDevice(device->key);
        return basaltDevice->vk().GetDeviceProcAddr(device, pName);
    }

    VK_LAYER_EXPORT PFN_vkVoidFunction VKAPI_CALL vkBasalt_GetInstanceProcAddr(VkInstance instance, const char* pName)
    {
        using namespace vkBasalt;

        auto lock = lockGlobally();

        auto layerFunc = interceptCalls(pName);
        if (layerFunc)
            return layerFunc;

        auto basaltInstance = getBasaltInstance(instance->key);
        return basaltInstance->vk().GetInstanceProcAddr(instance, pName);
    }
}
