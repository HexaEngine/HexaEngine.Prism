#pragma once
#include "../common.hpp"
#include "../prism.hpp"

#if HEXA_PRISM_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#if HEXA_PRISM_LINUX
#define VK_USE_PLATFORM_WAYLAND_KHR
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#if HEXA_PRISM_MACOS
#define VK_USE_PLATFORM_METAL_EXT
#endif

#include <vulkan/vulkan.h>