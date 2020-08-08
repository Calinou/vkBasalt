#pragma once

#include "vulkan_include.hpp"

typedef struct InstanceKey_T* InstanceKey;
typedef struct DeviceKey_T*   DeviceKey;

struct VkInstance_T
{
    InstanceKey key;
};

struct VkPhysicalDevice_T
{
    InstanceKey key;
};

struct VkDevice_T
{
    DeviceKey key;
};

struct VkQueue_T
{
    DeviceKey key;
};

struct VkCommandBuffer_T
{
    DeviceKey key;
};

namespace vkBasalt
{
    class VkBasaltDevice;

    class VkBasaltInstance
    {
    public:
        static VkResult createInstance(const VkInstanceCreateInfo*  pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator,
                                       PFN_vkGetInstanceProcAddr    gipa,
                                       VkBasaltInstance**           basaltInstance);

        static void destroyInstance(VkBasaltInstance* basaltInstance, const VkAllocationCallbacks* pAllocator);

        VkBasaltInstance(const VkBasaltInstance& other) = delete;
        VkBasaltInstance& operator=(const VkBasaltInstance& other) = delete;
        VkBasaltInstance(VkBasaltInstance&& other)                 = delete;
        VkBasaltInstance& operator=(VkBasaltInstance&& other) = delete;

        inline const VulkanDispatchTable& vk() const
        {
            return m_dispatch;
        }

        inline VkInstance get() const
        {
            return m_instance;
        }

        VkResult createDevice(VkPhysicalDevice             physDevice,
                              const VkDeviceCreateInfo*    pCreateInfo,
                              const VkAllocationCallbacks* pAllocator,
                              VkBasaltDevice**             basaltDevice) const;

        void destroyDevice(VkBasaltDevice* basaltDevice, const VkAllocationCallbacks* pAllocator) const;

    private:
        VulkanDispatchTable m_dispatch;
        const VkInstance    m_instance;

        VkBasaltInstance(VkInstance instance, PFN_vkGetInstanceProcAddr gipa);
        ~VkBasaltInstance();
    };

    class VkBasaltDevice
    {
    public:
        VkBasaltDevice(const VkBasaltDevice& other) = delete;
        VkBasaltDevice& operator=(const VkBasaltDevice& other) = delete;
        VkBasaltDevice(VkBasaltDevice&& other)                 = delete;
        VkBasaltDevice& operator=(VkBasaltDevice&& other) = delete;

        inline const VulkanDispatchTable& vk() const
        {
            return m_dispatch;
        }

        inline VkDevice get() const
        {
            return m_device;
        }

        inline const VkBasaltInstance* instance() const
        {
            return m_instance;
        }

        inline VkPhysicalDevice physDevice() const
        {
            return m_physDevice;
        }

    private:
        VulkanDispatchTable     m_dispatch;
        const VkBasaltInstance* m_instance;
        const VkDevice          m_device;
        const VkPhysicalDevice  m_physDevice;

        VkBasaltDevice(const VkBasaltInstance* basaltInstance, VkPhysicalDevice physDevice, VkDevice device);
        ~VkBasaltDevice();

        friend VkResult VkBasaltInstance::createDevice(VkPhysicalDevice             physDevice,
                                                       const VkDeviceCreateInfo*    pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkBasaltDevice**             basaltDevice) const;

        friend void VkBasaltInstance::destroyDevice(VkBasaltDevice* basaltDevice, const VkAllocationCallbacks* pAllocator) const;
    };
} // namespace vkBasalt
