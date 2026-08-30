#include "prism_asset.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

	namespace
	{
		AssetReadCallback g_assetReadCallback = nullptr;
		void* g_assetReadCallbackUserData = nullptr;
	}

	void SetAssetReadCallback(AssetReadCallback callback, void* userData)
	{
		g_assetReadCallback = callback;
		g_assetReadCallbackUserData = userData;
	}

	bool ReadAsset(const AssetPath& path, std::vector<uint8_t>& outData)
	{
		if (!g_assetReadCallback)
		{
			return false;
		}

		size_t size = 0;
		uint8_t* data = g_assetReadCallback(path.raw.c_str(), &size, g_assetReadCallbackUserData);
		if (!data)
		{
			return false;
		}

		outData.assign(data, data + size);
		FreeAssetData(data);
		return true;
	}

	void FreeAssetData(uint8_t* data)
	{
		PrismFree(data);
	}

HEXA_PRISM_NAMESPACE_END
