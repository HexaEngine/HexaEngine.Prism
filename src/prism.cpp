#include "prism.hpp"

#ifdef HEXA_PRISM_D3D11
#include "d3d11/d3d11.hpp"
#endif
#ifdef HEXA_PRISM_VULKAN
#include "vulkan/vulkan.hpp"
#endif

HEXA_PRISM_NAMESPACE_BEGIN

PrismObj<PrismDevice> PrismDevice::Create(const DeviceDesc& desc)
{
	switch (desc.type)
	{
#if defined(HEXA_PRISM_D3D11)
	case BackendType::D3D11:
	{
		D3D11GraphicsDevice* device = new D3D11GraphicsDevice();
		if (!device->Initialize(flags))
		{
			delete device;
			return {};
		}
		return PrismObj<GraphicsDevice>(device);
	}
	break;
#endif
#if defined(HEXA_PRISM_VULKAN)
	case BackendType::Vulkan:
	{
		auto device = MakePrismObj<VulkanDevice>();
		if (!device->Initialize(desc))
		{
			return {};
		}
		return device;
	}
	break;
#endif
	default:
		return {};
	}
}

HEXA_PRISM_NAMESPACE_END