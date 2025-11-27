#include "graphics.hpp"

#ifdef HEXA_PRISM_WINDOWS
#include "d3d11/d3d11.hpp"
#endif

HEXA_PRISM_NAMESPACE_BEGIN

PrismObj<GraphicsDevice> GraphicsDevice::Create()
{
#ifdef HEXA_PRISM_WINDOWS
	D3D11GraphicsDevice* device = new D3D11GraphicsDevice();
	if (!device->Initialize())
	{
		delete device;
		return {};
	}
	return PrismObj<GraphicsDevice>(device);
#else
	return {};
#endif
}

HEXA_PRISM_NAMESPACE_END