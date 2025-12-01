#include "d3d11/descriptor_range.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

	static D3D11ShaderParameter* Find(D3D11ShaderParameter* buckets, uint32_t capacity, uint32_t hash, const char* key)
	{
		uint32_t index = hash % capacity;
		bool exit = false;
		while (true)
		{
			auto entry = &buckets[index];
			if (entry->hash == 0)
			{
				return entry;
			}
			else if (entry->hash == hash && strcmp(key, entry->name) == 0)
			{
				return entry;
			}

			index++;
			if (index == capacity)
			{
				if (exit)
				{
					break;
				}

				index = 0;
				exit = true;
			}
		}

		return nullptr;
	}

	D3D11DescriptorRange::D3D11DescriptorRange(ShaderStage stage, ShaderParameterType type, const D3D11ShaderParameter* parameters, int parametersLength)
	{
		this->stage = stage;
		this->type = type;

		bucketCount = parametersLength;
		if (bucketCount > 0)
		{
			buckets = PrismAllocT<D3D11ShaderParameter>(bucketCount);
			PrismZeroMemoryT(buckets, bucketCount);
		}

		uint32_t startSlot = std::numeric_limits<uint32_t>::max();
		uint32_t maxSlot = 0;
		for (int i = 0; i < parametersLength; i++)
		{
			const D3D11ShaderParameter& parameter = parameters[i];
			startSlot = std::min(startSlot, parameter.index);
			maxSlot = std::max(maxSlot, parameter.index);
			auto param = Find(buckets, parametersLength, parameter.hash, parameter.name);
			*param = parameter;
		}

		uint32_t rangeWidth = parametersLength == 0 ? 0 : (maxSlot + 1) - startSlot;

		this->startSlot = startSlot;
		this->count = rangeWidth;

		if (bucketCount == 0)
		{
			return;
		}
		resources = PrismAllocT<void*>(rangeWidth);
		PrismZeroMemory(resources, rangeWidth * sizeof(void*));

		if (type == ShaderParameterType::UAV)
		{
			initialCounts = PrismAllocT<uint32_t>(rangeWidth);
			for (uint32_t i = 0; i < rangeWidth; i++)
			{
				initialCounts[i] = std::numeric_limits<uint32_t>::max();
			}
		}
	}

	D3D11DescriptorRange::~D3D11DescriptorRange()
	{
		if (resources != nullptr)
		{
			PrismFree(resources);
			resources = nullptr;
		}
		if (initialCounts != nullptr)
		{
			PrismFree(initialCounts);
			initialCounts = nullptr;
		}
		if (buckets != nullptr)
		{
			for (size_t i = 0; i < bucketCount; i++)
			{
				PrismFree(buckets[i].name);
			}
			PrismFree(buckets);
			buckets = nullptr;
			bucketCount = 0;
		}
	}

	D3D11ShaderParameter D3D11DescriptorRange::GetByName(const char* name) const
	{
		if (bucketCount == 0)
		{
			throw std::out_of_range("Key not found");
		}

		uint32_t hash = HashString(name);
		auto pEntry = Find(buckets, bucketCount, hash, name);
		if (pEntry != nullptr && pEntry->hash == hash)
		{
			return *pEntry;
		}
		throw std::out_of_range("Key not found");
	}

	bool D3D11DescriptorRange::TryGetByName(const char* name, D3D11ShaderParameter& parameter) const
	{
		if (bucketCount == 0)
		{
			parameter = {};
			return false;
		}

		uint32_t hash = HashString(name);
		auto pEntry = Find(buckets, bucketCount, hash, name);
		if (pEntry != nullptr && pEntry->hash == hash)
		{
			parameter = *pEntry;
			return true;
		}
		parameter = {};
		return false;
	}

	void D3D11DescriptorRange::SetByName(const char* name, void* resource)
	{
		auto parameter = GetByName(name);
		auto old = resources[parameter.index - startSlot];
		resources[parameter.index - startSlot] = resource;
		if ((old != nullptr) != (resource != nullptr))
		{
			UpdateRanges(parameter.index - startSlot, resource == nullptr);
		}
	}

	bool D3D11DescriptorRange::TrySetByName(const char* name, void* resource, uint32_t initialValue)
	{
		D3D11ShaderParameter parameter;
		if (TryGetByName(name, parameter))
		{
			auto index = parameter.index - startSlot;
			auto old = resources[index];
			resources[index] = resource;
			if (initialCounts != nullptr)
			{
				initialCounts[index] = initialValue;
			}
			if ((old != nullptr) != (resource != nullptr))
			{
				UpdateRanges(index, resource == nullptr);
			}

			return true;
		}
		return false;
	}

	void D3D11DescriptorRange::UpdateByName(const char* name, void* oldState, void* state, uint32_t initialValue)
	{
		D3D11ShaderParameter parameter;
		if (TryGetByName(name, parameter))
		{
			auto index = parameter.index - startSlot;
			auto old = resources[index];

			if (old != oldState)
			{
				return;
			}

			resources[index] = state;
			if (initialCounts != nullptr)
			{
				initialCounts[index] = initialValue;
			}
			if ((old != nullptr) != (state != nullptr))
			{
				UpdateRanges(index, state == nullptr);
			}
		}
	}

	void D3D11DescriptorRange::UpdateRanges(uint32_t idx, bool clear)
	{
		if (clear)
		{
			for (uint32_t i = 0; i < ranges.size(); i++)
			{
				ResourceRange* range = ranges.data() + i;
				uint32_t rangeStart = static_cast<uint32_t>(range->start - resources);
				uint32_t rangeEnd = rangeStart + range->length - 1;

				if (idx >= rangeStart && idx <= rangeEnd)
				{
					if (range->length == 1)
					{
						ranges.erase(ranges.begin() + i);
					}
					else if (idx == rangeStart)
					{
						range->start++;
						range->length--;
					}
					else if (idx == rangeEnd)
					{
						range->length--;
					}
					else
					{
						ResourceRange newRange{
							range->start + (idx - rangeStart + 1),
							rangeEnd - idx
						};

						range->length = idx - rangeStart;
						ranges.insert(ranges.begin() + i + 1, newRange);
					}
					return;
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < ranges.size(); i++)
			{
				ResourceRange* range = ranges.data() + i;
				uint32_t rangeStart = static_cast<uint32_t>(range->start - resources);
				uint32_t rangeEnd = rangeStart + range->length - 1;

				if (idx == rangeStart - 1)
				{
					range->start--;
					range->length++;

					if (i > 0)
					{
						ResourceRange* previousRange = ranges.data() + i - 1;
						if (previousRange->start + previousRange->length == range->start)
						{
							previousRange->length += range->length;
							ranges.erase(ranges.begin() + i);
						}
					}
					return;
				}
				else if (idx == rangeEnd + 1)
				{
					range->length++;

					if (i < ranges.size() - 1)
					{
						ResourceRange* nextRange = ranges.data() + i + 1;
						if (range->start + range->length == nextRange->start)
						{
							range->length += nextRange->length;
							ranges.erase(ranges.begin() + i + 1);
						}
					}
					return;
				}
				else if (idx < rangeStart)
				{
					ResourceRange newRange{ resources + idx, 1 };
					ranges.insert(ranges.begin() + i, newRange);

					if (i < ranges.size() - 1)
					{
						ResourceRange* nextRange = ranges.data() + i + 1;
						if (newRange.start + newRange.length == nextRange->start)
						{
							ranges[i].length += nextRange->length;
							ranges.erase(ranges.begin() + i + 1);
						}
					}
					return;
				}
			}

			ResourceRange endRange{ resources + idx, 1 };
			ranges.push_back(endRange);

			if (ranges.size() > 1)
			{
				ResourceRange* previousRange = ranges.data() + ranges.size() - 2;
				if (previousRange->start + previousRange->length == endRange.start)
				{
					previousRange->length += endRange.length;
					ranges.erase(ranges.begin() + ranges.size() - 1);
				}
			}
		}
	}

	void D3D11DescriptorRange::Bind(const ComPtr<ID3D11DeviceContext3>& context, BindCallback func) const
	{
		for (auto& range : ranges)
		{
			if (range.length > 0)
			{
				void** res = range.start;
				auto start = static_cast<uint32_t>(range.start - resources);
				func(context, startSlot + start, range.length, res);
			}
		}
	}

	void D3D11DescriptorRange::BindUAV(const ComPtr<ID3D11DeviceContext3>& context) const
	{
		for (auto& range : ranges)
		{
			if (range.length > 0)
			{
				void** res = range.start;
				auto start = static_cast<uint32_t>(range.start - resources);
				context->CSSetUnorderedAccessViews(startSlot + start, range.length, reinterpret_cast<ID3D11UnorderedAccessView**>(res), initialCounts);
			}
		}
	}

	void D3D11DescriptorRange::Unbind(const ComPtr<ID3D11DeviceContext3>& context, BindCallback func) const
	{
		void* nullResources[256] = {};
		for (auto& range : ranges)
		{
			if (range.length > 0)
			{
				auto start = static_cast<uint32_t>(range.start - resources);
				func(context, startSlot + start, range.length, nullResources);
			}
		}
	}

	void D3D11DescriptorRange::UnbindUAV(const ComPtr<ID3D11DeviceContext3>& context) const
	{
		void* nullResources[256] = {};
		for (auto& range : ranges)
		{
			if (range.length > 0)
			{
				auto start = static_cast<uint32_t>(range.start - resources);
				context->CSSetUnorderedAccessViews(startSlot + start, range.length, reinterpret_cast<ID3D11UnorderedAccessView**>(nullResources), initialCounts + start);
			}
		}
	}

HEXA_PRISM_NAMESPACE_END
