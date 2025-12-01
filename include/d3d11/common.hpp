#pragma once
#include "../common.hpp"
#include "../prism.hpp"

#if !HEXA_PRISM_WINDOWS
#error "This file is only for Windows platform."
#endif

#define NOMINMAX 1
#include <windows.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

HEXA_PRISM_NAMESPACE_BEGIN

using Microsoft::WRL::ComPtr;

HEXA_PRISM_NAMESPACE_END