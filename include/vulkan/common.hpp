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
#include <vk_mem_alloc.h>

HEXA_PRISM_NAMESPACE_BEGIN

struct VulkanQueueStore : PrismObject
{
	VkQueue queue = VK_NULL_HANDLE;
	uint32_t familyIndex = static_cast<uint32_t>(-1);
	uint32_t queueIndex = static_cast<uint32_t>(-1);
	Utils::fmutex mutex;

	VulkanQueueStore() = default;

	VulkanQueueStore(VkQueue queue, uint32_t familyIndex, uint32_t queueIndex) : queue(queue), familyIndex(familyIndex), queueIndex(queueIndex)
	{
	}

	VulkanQueueStore(VkDevice device, uint32_t familyIndex, uint32_t queueIndex) : queue(VK_NULL_HANDLE), familyIndex(familyIndex), queueIndex(queueIndex)
	{
		vkGetDeviceQueue(device, familyIndex, queueIndex, &queue);
	}
};

HEXA_PRISM_NAMESPACE_END