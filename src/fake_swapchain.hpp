#ifndef FAKE_SWAPCHAIN_HPP_INCLUDED
#define FAKE_SWAPCHAIN_HPP_INCLUDED
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <memory>

#include "vulkan_include.hpp"

#include "logical_device.hpp"

#include "image.hpp"

namespace vkBasalt
{
    void
    createFakeSwapchainImages2(LogicalDevice* pLogicalDevice, VkSwapchainCreateInfoKHR* pSwapchainCreateInfo, uint32_t count, VkBasaltImage* pImages);

    std::vector<VkImage> createFakeSwapchainImages(LogicalDevice*           pLogicalDevice,
                                                   VkSwapchainCreateInfoKHR swapchainCreateInfo,
                                                   uint32_t                 count,
                                                   VkDeviceMemory&          deviceMemory);
} // namespace vkBasalt

#endif // FAKE_SWAPCHAIN_HPP_INCLUDED
