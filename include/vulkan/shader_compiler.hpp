#pragma once
#include "common.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

class VulkanShaderCompiler
{
public:
	static bool Compile(ShaderSource* source, const char* entryPoint, ShaderStage stage, PrismObj<Blob>& shaderOut);
};

HEXA_PRISM_NAMESPACE_END
