#include "prism_texture_loader.hpp"
#include <cctype>

HEXA_PRISM_NAMESPACE_BEGIN

	namespace
	{
		TexFileFormat FormatFromExtension(std::string_view filename)
		{
			size_t dot = filename.find_last_of('.');
			if (dot == std::string_view::npos)
			{
				return TexFileFormat::WIC;
			}

			std::string ext(filename.substr(dot));
			for (char& c : ext)
			{
				c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			}

			if (ext == ".dds") return TexFileFormat::DDS;
			if (ext == ".tga") return TexFileFormat::TGA;
			if (ext == ".hdr") return TexFileFormat::HDR;
			return TexFileFormat::WIC;
		}

		Format ResolveFormat(DXGI_FORMAT format, bool forceSRGB)
		{
			return static_cast<Format>(forceSRGB ? DirectX::MakeSRGB(format) : format);
		}

		// Flattens a ScratchImage into Prism's array-major/mip-minor SubresourceData convention
		// (see PrismDevice::CreateTexture1D/2D/3D). For 3D, one entry per mip level - DirectXTex
		// lays every depth slice of a mip out contiguously, so GetImage(mip,0,0)'s pixel pointer
		// plus slicePitch stepping is exactly what D3D's per-mip Texture3D subresource expects.
		void BuildSubresourceData(const DirectX::ScratchImage& image, bool is3D, std::vector<SubresourceData>& out)
		{
			const DirectX::TexMetadata& metadata = image.GetMetadata();
			out.clear();

			if (is3D)
			{
				out.reserve(metadata.mipLevels);
				for (size_t mip = 0; mip < metadata.mipLevels; ++mip)
				{
					const DirectX::Image* img = image.GetImage(mip, 0, 0);
					out.push_back(SubresourceData{ img->pixels, static_cast<uint32_t>(img->rowPitch), static_cast<uint32_t>(img->slicePitch) });
				}
			}
			else
			{
				out.reserve(metadata.arraySize * metadata.mipLevels);
				for (size_t item = 0; item < metadata.arraySize; ++item)
				{
					for (size_t mip = 0; mip < metadata.mipLevels; ++mip)
					{
						const DirectX::Image* img = image.GetImage(mip, item, 0);
						out.push_back(SubresourceData{ img->pixels, static_cast<uint32_t>(img->rowPitch), static_cast<uint32_t>(img->slicePitch) });
					}
				}
			}
		}
	}

	bool TextureLoader::LoadFromMemory(TexFileFormat format, const uint8_t* data, size_t length, DirectX::ScratchImage& outImage) const
	{
		HRESULT hr;
		switch (format)
		{
		case TexFileFormat::DDS:
			hr = DirectX::LoadFromDDSMemory(data, length, DirectX::DDS_FLAGS_NONE, nullptr, outImage);
			break;
		case TexFileFormat::TGA:
			hr = DirectX::LoadFromTGAMemory(data, length, DirectX::TGA_FLAGS_NONE, nullptr, outImage);
			break;
		case TexFileFormat::HDR:
			hr = DirectX::LoadFromHDRMemory(data, length, nullptr, outImage);
			break;
		case TexFileFormat::WIC:
		case TexFileFormat::Auto:
		default:
#ifdef _WIN32
			hr = DirectX::LoadFromWICMemory(data, length, DirectX::WIC_FLAGS_NONE, nullptr, outImage);
#else
			// WIC (BMP/PNG/JPEG/TIFF/GIF/...) is Windows-only; DDS/TGA/HDR work everywhere.
			return false;
#endif
			break;
		}
		return SUCCEEDED(hr);
	}

	bool TextureLoader::LoadFromMemory(const std::string& filenameHint, const uint8_t* data, size_t length, DirectX::ScratchImage& outImage) const
	{
		return LoadFromMemory(FormatFromExtension(filenameHint), data, length, outImage);
	}

	bool TextureLoader::LoadFromFile(const std::string& filename, DirectX::ScratchImage& outImage) const
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (!file)
		{
			return false;
		}

		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<uint8_t> data(static_cast<size_t>(size));
		if (!file.read(reinterpret_cast<char*>(data.data()), size))
		{
			return false;
		}

		return LoadFromMemory(filename, data.data(), data.size(), outImage);
	}

	bool TextureLoader::LoadFromAssets(const AssetPath& path, DirectX::ScratchImage& outImage) const
	{
		std::vector<uint8_t> data;
		if (!ReadAsset(path, data))
		{
			return false;
		}
		return LoadFromMemory(std::string(path.GetPath()), data.data(), data.size(), outImage);
	}

	bool TextureLoader::Load(const AssetPath& path, DirectX::ScratchImage& outImage) const
	{
		return path.HasNamespace() ? LoadFromAssets(path, outImage) : LoadFromFile(path.raw, outImage);
	}

	void TextureLoader::ApplyLoaderFlags(DirectX::ScratchImage& image) const
	{
		if ((flags & TextureLoaderFlags::Scale) != TextureLoaderFlags::None && scalingFactor != 1.0f)
		{
			const DirectX::TexMetadata& metadata = image.GetMetadata();
			size_t newWidth = (std::max)(static_cast<size_t>(1), static_cast<size_t>(static_cast<float>(metadata.width) * scalingFactor));
			size_t newHeight = (std::max)(static_cast<size_t>(1), static_cast<size_t>(static_cast<float>(metadata.height) * scalingFactor));

			DirectX::ScratchImage resized;
			if (SUCCEEDED(DirectX::Resize(image.GetImages(), image.GetImageCount(), metadata, newWidth, newHeight, DirectX::TEX_FILTER_DEFAULT, resized)))
			{
				image = std::move(resized);
			}
		}

		if ((flags & TextureLoaderFlags::GenerateMipMaps) != TextureLoaderFlags::None)
		{
			const DirectX::TexMetadata& metadata = image.GetMetadata();
			if (metadata.mipLevels == 1 && metadata.width > 1 && metadata.height > 1)
			{
				DirectX::ScratchImage mipped;
				HRESULT hr = metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE3D
					? DirectX::GenerateMipMaps3D(image.GetImages(), image.GetImageCount(), metadata, DirectX::TEX_FILTER_DEFAULT, 0, mipped)
					: DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), metadata, DirectX::TEX_FILTER_DEFAULT, 0, mipped);
				if (SUCCEEDED(hr))
				{
					image = std::move(mipped);
				}
			}
		}
	}

	DirectX::ScratchImage TextureLoader::MakeFallbackImage(TextureDimension dimension) const
	{
		DirectX::ScratchImage fallback;
		switch (dimension)
		{
		case TextureDimension::Texture1D:
			fallback.Initialize1D(DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 1, 1, DirectX::CP_FLAGS_NONE);
			break;
		case TextureDimension::Texture3D:
			fallback.Initialize3D(DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 1, 1, 1, DirectX::CP_FLAGS_NONE);
			break;
		case TextureDimension::TextureCube:
			fallback.InitializeCube(DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 1, 1, 1, DirectX::CP_FLAGS_NONE);
			break;
		case TextureDimension::Texture2D:
		default:
			fallback.Initialize2D(DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 1, 1, 1, DirectX::CP_FLAGS_NONE);
			break;
		}

		float* pixels = reinterpret_cast<float*>(fallback.GetPixels());
		pixels[0] = 1.0f; pixels[1] = 0.0f; pixels[2] = 1.0f; pixels[3] = 1.0f; // magenta

		return fallback;
	}

	PrismObj<Texture1D> TextureLoader::CreateTexture1DFromImage(const DirectX::ScratchImage& image, const TextureFileDescription& desc) const
	{
		const DirectX::TexMetadata& metadata = image.GetMetadata();

		std::vector<SubresourceData> subresources;
		BuildSubresourceData(image, false, subresources);

		Texture1DDesc texDesc = {};
		texDesc.gpuAccessFlags = desc.gpuAccessFlags;
		texDesc.cpuAccessFlags = desc.cpuAccessFlags;
		texDesc.format = ResolveFormat(metadata.format, desc.forceSRGB);
		texDesc.width = static_cast<uint32_t>(metadata.width);
		texDesc.arraySize = static_cast<uint32_t>(metadata.arraySize);
		texDesc.mipLevels = static_cast<uint32_t>(metadata.mipLevels);
		texDesc.miscFlags = desc.miscFlags;

		return device->CreateTexture1D(texDesc, subresources.data());
	}

	PrismObj<Texture2D> TextureLoader::CreateTexture2DFromImage(const DirectX::ScratchImage& image, const TextureFileDescription& desc) const
	{
		const DirectX::TexMetadata& metadata = image.GetMetadata();

		std::vector<SubresourceData> subresources;
		BuildSubresourceData(image, false, subresources);

		Texture2DDesc texDesc = {};
		texDesc.gpuAccessFlags = desc.gpuAccessFlags;
		texDesc.cpuAccessFlags = desc.cpuAccessFlags;
		texDesc.format = ResolveFormat(metadata.format, desc.forceSRGB);
		texDesc.width = static_cast<uint32_t>(metadata.width);
		texDesc.height = static_cast<uint32_t>(metadata.height);
		texDesc.arraySize = static_cast<uint32_t>(metadata.arraySize);
		texDesc.mipLevels = static_cast<uint32_t>(metadata.mipLevels);
		texDesc.sampleDesc = { 1, 0 };
		texDesc.miscFlags = desc.miscFlags;
		if (metadata.IsCubemap())
		{
			texDesc.miscFlags = texDesc.miscFlags | ResourceMiscFlags::TextureCube;
		}

		return device->CreateTexture2D(texDesc, subresources.data());
	}

	PrismObj<Texture3D> TextureLoader::CreateTexture3DFromImage(const DirectX::ScratchImage& image, const TextureFileDescription& desc) const
	{
		const DirectX::TexMetadata& metadata = image.GetMetadata();

		std::vector<SubresourceData> subresources;
		BuildSubresourceData(image, true, subresources);

		Texture3DDesc texDesc = {};
		texDesc.gpuAccessFlags = desc.gpuAccessFlags;
		texDesc.cpuAccessFlags = desc.cpuAccessFlags;
		texDesc.format = ResolveFormat(metadata.format, desc.forceSRGB);
		texDesc.width = static_cast<uint32_t>(metadata.width);
		texDesc.height = static_cast<uint32_t>(metadata.height);
		texDesc.depth = static_cast<uint32_t>(metadata.depth);
		texDesc.mipLevels = static_cast<uint32_t>(metadata.mipLevels);
		texDesc.miscFlags = desc.miscFlags;

		return device->CreateTexture3D(texDesc, subresources.data());
	}

	PrismObj<Texture1D> TextureLoader::LoadTexture1D(const TextureFileDescription& desc) const
	{
		DirectX::ScratchImage image;
		if (!Load(desc.path, image))
		{
			image = MakeFallbackImage(TextureDimension::Texture1D);
		}
		else
		{
			ApplyLoaderFlags(image);
		}
		return CreateTexture1DFromImage(image, desc);
	}

	PrismObj<Texture2D> TextureLoader::LoadTexture2D(const TextureFileDescription& desc) const
	{
		DirectX::ScratchImage image;
		if (!Load(desc.path, image))
		{
			image = MakeFallbackImage(desc.dimension);
		}
		else
		{
			ApplyLoaderFlags(image);
		}
		return CreateTexture2DFromImage(image, desc);
	}

	PrismObj<Texture3D> TextureLoader::LoadTexture3D(const TextureFileDescription& desc) const
	{
		DirectX::ScratchImage image;
		if (!Load(desc.path, image))
		{
			image = MakeFallbackImage(TextureDimension::Texture3D);
		}
		else
		{
			ApplyLoaderFlags(image);
		}
		return CreateTexture3DFromImage(image, desc);
	}

	PrismObj<Texture1D> TextureLoader::LoadTexture1D(const AssetPath& path, GpuAccessFlags gpuAccessFlags, CpuAccessFlags cpuAccessFlags, ResourceMiscFlags miscFlags) const
	{
		TextureFileDescription desc = {};
		desc.path = path;
		desc.dimension = TextureDimension::Texture1D;
		desc.gpuAccessFlags = gpuAccessFlags;
		desc.cpuAccessFlags = cpuAccessFlags;
		desc.miscFlags = miscFlags;
		return LoadTexture1D(desc);
	}

	PrismObj<Texture2D> TextureLoader::LoadTexture2D(const AssetPath& path, GpuAccessFlags gpuAccessFlags, CpuAccessFlags cpuAccessFlags, ResourceMiscFlags miscFlags) const
	{
		TextureFileDescription desc = {};
		desc.path = path;
		desc.dimension = TextureDimension::Texture2D;
		desc.gpuAccessFlags = gpuAccessFlags;
		desc.cpuAccessFlags = cpuAccessFlags;
		desc.miscFlags = miscFlags;
		return LoadTexture2D(desc);
	}

	PrismObj<Texture3D> TextureLoader::LoadTexture3D(const AssetPath& path, GpuAccessFlags gpuAccessFlags, CpuAccessFlags cpuAccessFlags, ResourceMiscFlags miscFlags) const
	{
		TextureFileDescription desc = {};
		desc.path = path;
		desc.dimension = TextureDimension::Texture3D;
		desc.gpuAccessFlags = gpuAccessFlags;
		desc.cpuAccessFlags = cpuAccessFlags;
		desc.miscFlags = miscFlags;
		return LoadTexture3D(desc);
	}

	PrismObj<Texture1D> TextureLoader::LoadTexture1D(const AssetPath& path) const
	{
		return LoadTexture1D(path, GpuAccessFlags::Read, CpuAccessFlags::None, ResourceMiscFlags::None);
	}

	PrismObj<Texture2D> TextureLoader::LoadTexture2D(const AssetPath& path) const
	{
		return LoadTexture2D(path, GpuAccessFlags::Read, CpuAccessFlags::None, ResourceMiscFlags::None);
	}

	PrismObj<Texture3D> TextureLoader::LoadTexture3D(const AssetPath& path) const
	{
		return LoadTexture3D(path, GpuAccessFlags::Read, CpuAccessFlags::None, ResourceMiscFlags::None);
	}

HEXA_PRISM_NAMESPACE_END
