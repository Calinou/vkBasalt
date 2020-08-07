#pragma once

#include <cassert>
#include <string>

#include "logger.hpp"

#define VK_NO_PROTOTYPES 1

#include "vulkan/vulkan.h"
#include "vulkan/vk_layer.h"

#include "generated/vulkan_dispatch_table.hpp"

#ifndef ASSERT_VULKAN
#define ASSERT_VULKAN(val)                                                                                                           \
    if (val != VK_SUCCESS)                                                                                                           \
    {                                                                                                                                \
        Logger::err(std::string(__FILE__) + ": " + std::to_string(__LINE__) + ": ASSERT_VULKAN failed with " + std::to_string(val)); \
        assert(0);                                                                                                                   \
    }
#endif
