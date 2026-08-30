#include "prism_asset.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

	namespace
	{
		AssetReadCallback g_assetReadCallback = nullptr;
		AssetFreeCallback g_assetFreeCallback = nullptr;
		void* g_assetCallbackUserData = nullptr;
	}

	void SetAssetReadCallback(AssetReadCallback readCallback, AssetFreeCallback freeCallback, void* userData)
	{
		g_assetReadCallback = readCallback;
		g_assetFreeCallback = freeCallback;
		g_assetCallbackUserData = userData;
	}

	bool ReadAsset(const AssetPath& path, std::vector<uint8_t>& outData)
	{
		if (!g_assetReadCallback || !g_assetFreeCallback)
		{
			return false;
		}

		size_t size = 0;
		uint8_t* data = g_assetReadCallback(path.raw.c_str(), &size, g_assetCallbackUserData);
		if (!data)
		{
			return false;
		}

		outData.assign(data, data + size);
		g_assetFreeCallback(data, g_assetCallbackUserData);
		return true;
	}

HEXA_PRISM_NAMESPACE_END
