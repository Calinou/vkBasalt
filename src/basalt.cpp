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

    static std::unordered_map<void*, VkBasaltInstance*> g_instanceMap;
    static std::unordered_map<void*, VkBasaltDevice*>   g_deviceMap;

    static inline VkBasaltInstance* getBasaltInstance(VkInstance instance)
    {
        return g_instanceMap[*(void**) instance];
    }

    static inline VkBasaltInstance* getBasaltInstance(VkPhysicalDevice physDevice)
    {
        return getBasaltInstance((VkInstance) physDevice);
    }

    static inline VkBasaltDevice* getBasaltDevice(VkDevice device)
    {
        return g_deviceMap[*(void**) device];
    }

    static inline VkBasaltDevice* getBasaltDevice(VkQueue queue)
    {
        return getBasaltDevice((VkDevice) queue);
    }

    static inline VkBasaltDevice* getBasaltDevice(VkCommandBuffer comBuffer)
    {
        return getBasaltDevice((VkDevice) comBuffer);
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

        VkResult ret = VkBasaltInstance::createInstance(pCreateInfo, pAllocator, gipa, &basaltInstance);
        if (ret != VK_SUCCESS)
            return ret;

        *pInstance = basaltInstance->instance;

        g_instanceMap[*(void**) *pInstance] = basaltInstance;

        return VK_SUCCESS;
    }

    void VKAPI_CALL vkBasalt_DestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator)
    {
        Logger::trace("vkDestroyInstance");
        auto lock = lockGlobally();

        auto basaltInstance = getBasaltInstance(instance);

        g_instanceMap.erase(*(void**) instance);

        VkBasaltInstance::destroyInstance(basaltInstance, pAllocator);
    }

    VkResult VKAPI_CALL vkBasalt_CreateDevice(VkPhysicalDevice             physicalDevice,
                                              const VkDeviceCreateInfo*    pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator,
                                              VkDevice*                    pDevice)
    {
        Logger::trace("vkCreateDevice");
        auto lock = lockGlobally();

        auto basaltInstance = getBasaltInstance(physicalDevice);

        auto layerCreateInfo = pNextSearch<VkLayerDeviceCreateInfo>(pCreateInfo->pNext, [](auto x) { return x->function == VK_LAYER_LINK_INFO; });

        if (layerCreateInfo == nullptr)
            return VK_ERROR_INITIALIZATION_FAILED;

        // move chain on for next layer
        layerCreateInfo->u.pLayerInfo = layerCreateInfo->u.pLayerInfo->pNext;

        VkBasaltDevice* basaltDevice;

        VkResult ret = basaltInstance->createDevice(physicalDevice, pCreateInfo, pAllocator, &basaltDevice);
        if (ret != VK_SUCCESS)
            return ret;

        *pDevice = basaltDevice->device;

        g_deviceMap[*(void**) *pDevice] = basaltDevice;

        return VK_SUCCESS;
    }

    void VKAPI_CALL vkBasalt_DestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
    {
        Logger::trace("vkDestroyDevice");
        auto lock = lockGlobally();

        auto basaltDevice = getBasaltDevice(device);

        g_deviceMap.erase(*(void**) device);

        basaltDevice->instance->destroyDevice(basaltDevice, pAllocator);
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
        // GETPROCADDR(EnumerateInstanceLayerProperties);
        // GETPROCADDR(EnumerateInstanceExtensionProperties);
        GETPROCADDR(CreateInstance);
        GETPROCADDR(DestroyInstance);

        /* device chain functions we intercept*/
        GETPROCADDR(GetDeviceProcAddr);
        // GETPROCADDR(EnumerateDeviceLayerProperties);
        // GETPROCADDR(EnumerateDeviceExtensionProperties);
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

        auto basaltDevice = getBasaltDevice(device);
        return basaltDevice->vk.GetDeviceProcAddr(device, pName);
    }

    VK_LAYER_EXPORT PFN_vkVoidFunction VKAPI_CALL vkBasalt_GetInstanceProcAddr(VkInstance instance, const char* pName)
    {
        using namespace vkBasalt;

        auto lock = lockGlobally();

        auto layerFunc = interceptCalls(pName);
        if (layerFunc)
            return layerFunc;

        auto basaltInstance = getBasaltInstance(instance);
        return basaltInstance->vk.GetInstanceProcAddr(instance, pName);
    }
}
