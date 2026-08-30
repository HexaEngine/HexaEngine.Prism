#pragma once
#include "prism.hpp"
#include "prism_asset.hpp"
#include <DirectXTex.h>

HEXA_PRISM_NAMESPACE_BEGIN

	// Mirrors HexaEngine.Core.Graphics.TextureDimension: which PrismDevice::CreateTextureXD
	// call (and, for the fallback texture, which shape) a load should target. Distinct from
	// DirectX::TEX_DIMENSION, which has no explicit cube-map value (cube is a metadata misc
	// flag on top of TEX_DIMENSION_TEXTURE2D there).
	enum class TextureDimension
	{
		Unknown,
		Texture1D,
		Texture2D,
		Texture3D,
		TextureCube,
	};

	// DirectXTex dispatches file/memory loads to one of exactly these paths by extension;
	// WIC (Windows Imaging Component, Windows-only) covers BMP/JPEG/PNG/TIFF/GIF/WMP/ICO.
	enum class TexFileFormat
	{
		Auto,
		DDS,
		TGA,
		HDR,
		WIC,
	};

	enum class TextureLoaderFlags : uint32_t
	{
		None = 0,
		GenerateMipMaps = 1 << 0,
		Scale = 1 << 1,
	};
	HEXA_PRISM_DEFINE_FLAG_OPERATORS(TextureLoaderFlags);

	struct TextureFileDescription
	{
		AssetPath path;
		TextureDimension dimension = TextureDimension::Texture2D;
		uint32_t mipLevels = 0;
		GpuAccessFlags gpuAccessFlags = GpuAccessFlags::Read;
		CpuAccessFlags cpuAccessFlags = CpuAccessFlags::None;
		ResourceMiscFlags miscFlags = ResourceMiscFlags::None;
		bool forceSRGB = false;
	};

	// Loads images (DDS/TGA/HDR/WIC, via DirectXTex) from a plain filesystem path, an AssetPath
	// (routed through the registered asset-read callback, see prism_asset.hpp), or memory, and
	// creates Prism textures from them. Backend-agnostic: works with any PrismDevice, since it
	// only uses PrismDevice::CreateTexture1D/2D/3D's initial-data upload path.
	class TextureLoader
	{
	public:
		explicit TextureLoader(PrismDevice* device) : device(device) {}

		PrismDevice* GetDevice() const { return device; }

		TextureLoaderFlags GetFlags() const { return flags; }
		void SetFlags(TextureLoaderFlags value) { flags = value; }

		float GetScalingFactor() const { return scalingFactor; }
		void SetScalingFactor(float value) { scalingFactor = value; }

		// Raw image loading (no texture creation). Returns false (image left empty) on failure.
		bool LoadFromFile(const std::string& filename, DirectX::ScratchImage& outImage) const;
		bool LoadFromAssets(const AssetPath& path, DirectX::ScratchImage& outImage) const;
		bool LoadFromMemory(const std::string& filenameHint, const uint8_t* data, size_t length, DirectX::ScratchImage& outImage) const;
		bool LoadFromMemory(TexFileFormat format, const uint8_t* data, size_t length, DirectX::ScratchImage& outImage) const;

		// Convenience overload picking plain-filesystem vs. asset-namespace loading from the path.
		bool Load(const AssetPath& path, DirectX::ScratchImage& outImage) const;

		PrismObj<Texture1D> LoadTexture1D(const TextureFileDescription& desc) const;
		PrismObj<Texture2D> LoadTexture2D(const TextureFileDescription& desc) const;
		PrismObj<Texture3D> LoadTexture3D(const TextureFileDescription& desc) const;

		PrismObj<Texture1D> LoadTexture1D(const AssetPath& path, GpuAccessFlags gpuAccessFlags, CpuAccessFlags cpuAccessFlags, ResourceMiscFlags miscFlags) const;
		PrismObj<Texture2D> LoadTexture2D(const AssetPath& path, GpuAccessFlags gpuAccessFlags, CpuAccessFlags cpuAccessFlags, ResourceMiscFlags miscFlags) const;
		PrismObj<Texture3D> LoadTexture3D(const AssetPath& path, GpuAccessFlags gpuAccessFlags, CpuAccessFlags cpuAccessFlags, ResourceMiscFlags miscFlags) const;

		PrismObj<Texture1D> LoadTexture1D(const AssetPath& path) const;
		PrismObj<Texture2D> LoadTexture2D(const AssetPath& path) const;
		PrismObj<Texture3D> LoadTexture3D(const AssetPath& path) const;

	private:
		void ApplyLoaderFlags(DirectX::ScratchImage& image) const;
		PrismObj<Texture1D> CreateTexture1DFromImage(const DirectX::ScratchImage& image, const TextureFileDescription& desc) const;
		PrismObj<Texture2D> CreateTexture2DFromImage(const DirectX::ScratchImage& image, const TextureFileDescription& desc) const;
		PrismObj<Texture3D> CreateTexture3DFromImage(const DirectX::ScratchImage& image, const TextureFileDescription& desc) const;
		DirectX::ScratchImage MakeFallbackImage(TextureDimension dimension) const;

		PrismDevice* device;
		TextureLoaderFlags flags = TextureLoaderFlags::None;
		float scalingFactor = 1.0f;
	};

HEXA_PRISM_NAMESPACE_END
