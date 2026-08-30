#pragma once
#include "prism_base.hpp"
#include <string_view>

HEXA_PRISM_NAMESPACE_BEGIN

	// Mirrors HexaEngine.Core.IO.AssetPath: an optional "namespace:" prefix (e.g. "assets:foo.dds")
	// routes the path through the registered asset-read callback instead of the plain filesystem.
	// A single-character prefix (e.g. a Windows drive letter "C:") is not treated as a namespace.
	struct AssetPath
	{
		std::string raw;

		AssetPath() = default;
		AssetPath(std::string path) : raw(std::move(path)) {}
		AssetPath(const char* path) : raw(path) {}

		std::string_view GetNamespace() const
		{
			size_t sep = FindSeparator();
			return sep == std::string::npos ? std::string_view{} : std::string_view(raw).substr(0, sep);
		}

		std::string_view GetPath() const
		{
			size_t sep = FindSeparator();
			return sep == std::string::npos ? std::string_view(raw) : std::string_view(raw).substr(sep + 1);
		}

		bool HasNamespace() const { return FindSeparator() != std::string::npos; }

		bool operator==(const AssetPath& other) const { return raw == other.raw; }
		bool operator!=(const AssetPath& other) const { return raw != other.raw; }

	private:
		size_t FindSeparator() const
		{
			size_t sep = raw.find(':');
			return sep == 1 ? std::string::npos : sep;
		}
	};

	// Reads the full contents of an asset path, returning nullptr if not found. Prism has no
	// virtual filesystem of its own; the host application proxies asset reads through this
	// callback (mirroring HexaEngine.Core.IO.FileSystem.TryOpenRead). Plain C function pointer,
	// not std::function - must stay P/Invoke-marshalable from C# bindings. On success the
	// callback allocates the returned buffer via PrismAlloc and writes its length to *outSize;
	// the caller releases it with FreeAssetData().
	using AssetReadCallback = uint8_t*(*)(const char* path, size_t* outSize, void* userData);

	void SetAssetReadCallback(AssetReadCallback callback, void* userData = nullptr);
	bool ReadAsset(const AssetPath& path, std::vector<uint8_t>& outData);
	void FreeAssetData(uint8_t* data);

HEXA_PRISM_NAMESPACE_END
