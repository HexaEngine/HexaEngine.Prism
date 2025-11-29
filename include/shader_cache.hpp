#pragma once
#include "graphics.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

class ShaderCache
{
	struct ShaderCacheEntry
	{
		std::atomic<size_t> lock;
		char* key;
		PrismObj<Blob> shader;


	};

	std::atomic<size_t> lock;

public:
	ShaderCache() = default;
	~ShaderCache() = default;
	PrismObj<Blob> GetShader(const char* key) const;
	void SetShader(const char* key, Blob* shader);

private:
	void BeginRead();
	void EndRead();
	void BeginWrite();
	void EndWrite();
};

HEXA_PRISM_NAMESPACE_END