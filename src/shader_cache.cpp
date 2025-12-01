#include "shader_cache.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

PrismObj<Blob> ShaderCache::GetShader(const char* key) const
{
	return PrismObj<Blob>();
}

void ShaderCache::SetShader(const char* key, Blob* shader)
{

}

void ShaderCache::BeginRead()
{
}

void ShaderCache::EndRead()
{
}

void ShaderCache::BeginWrite()
{
}

void ShaderCache::EndWrite()
{
}

HEXA_PRISM_NAMESPACE_END