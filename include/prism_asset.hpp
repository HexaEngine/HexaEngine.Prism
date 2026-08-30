#pragma once
#include "prism_base.hpp"
#include "utils/span.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

	using HEXA_UTILS_NAMESPACE::StringSpan;
	using HEXA_UTILS_NAMESPACE::Index;

	struct AssetPath
	{
		String raw;

		AssetPath() = default;
		AssetPath(const char* path) : raw(path) {}

		StringSpan GetNamespace() const
		{
			auto sep = FindSeparator();
			return sep.IsInvalid() ? StringSpan{} : StringSpan(raw.data(), raw.size()).slice(0, sep.value);
		}

		StringSpan GetPath() const
		{
			auto sep = FindSeparator();
			return sep.IsInvalid() ? StringSpan(raw.data(), raw.size()) : StringSpan(raw.data(), raw.size()).slice(sep.value + 1);
		}

		bool HasNamespace() const { return FindSeparator().IsValid(); }

		bool operator==(const AssetPath& other) const { return StringSpan(raw.data(), raw.size()) == StringSpan(other.raw.data(), other.raw.size()); }
		bool operator!=(const AssetPath& other) const { return !(*this == other); }

	private:
		Index FindSeparator() const
		{
			auto sep = StringSpan(raw.data(), raw.size()).find(':');
			return sep.IsValid() && sep.value == 1 ? Index::Invalid() : sep;
		}
	};

	using AssetReadCallback = uint8_t*(*)(const char* path, size_t* outSize, void* userData);
	using AssetFreeCallback = void(*)(uint8_t* data, void* userData);

	void SetAssetReadCallback(AssetReadCallback readCallback, AssetFreeCallback freeCallback, void* userData = nullptr);
	bool ReadAsset(const AssetPath& path, std::vector<uint8_t>& outData);

HEXA_PRISM_NAMESPACE_END
