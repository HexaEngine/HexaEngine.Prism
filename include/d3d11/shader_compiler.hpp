#pragma once
#include "common.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

class D3D11ShaderCompiler
{
public:
	static bool Compile(ShaderSource* source, const char* entryPoint, const char* profile, PrismObj<Blob>& shaderOut);
};

HEXA_PRISM_NAMESPACE_END