#pragma once
#include "graphics.hpp"

#include <d3d11_4.h>
#include <wrl/client.h>

HEXA_PRISM_NAMESPACE_BEGIN

using Microsoft::WRL::ComPtr;

struct D3D11ShaderParameter
{
	char* name;
    uint32_t hash;
    uint32_t index;
    uint32_t size;
    ShaderStage stage;
    ShaderParameterType type;
};

struct D3D11DescriptorRange
{
    struct ResourceRange
    {
        void** start;
        uint32_t length;
    };

    D3D11DescriptorRange() = default;
	D3D11DescriptorRange(ShaderStage stage, ShaderParameterType type, const D3D11ShaderParameter* parameters, int parametersLength);
    ~D3D11DescriptorRange();

    ShaderStage stage;
    ShaderParameterType type;
    uint32_t startSlot;
    uint32_t count;
    void** resources;
    uint32_t* initialCounts;

    std::vector<ResourceRange> ranges;
    D3D11ShaderParameter* buckets;
    uint32_t bucketCount;

    static uint32_t HashString(const char* str)
    {
        uint32_t hash = 0x811c9dc5u;
        while (*str)
        {
            hash ^= static_cast<uint32_t>(*str++);
            hash *= 0x01000193u;
        }
        return hash;
    }

    D3D11ShaderParameter GetByName(const char* name) const;

    bool TryGetByName(const char* name, D3D11ShaderParameter& parameter) const;

    void SetByName(const char* name, void* resource);

    bool TrySetByName(const char* name, void* resource, uint32_t initialValue = static_cast<uint32_t>(-1));

    void UpdateByName(const char* name, void* oldState, void* state, uint32_t initialValue = static_cast<uint32_t>(-1));

    void UpdateRanges(uint32_t idx, bool clear);

    using BindCallback = void(*)(const ComPtr<ID3D11DeviceContext3>& context, uint32_t startSlot, uint32_t count, void** resources);

    void Bind(const ComPtr<ID3D11DeviceContext3>& context, BindCallback func) const;

    void BindUAV(const ComPtr<ID3D11DeviceContext3>& context) const;

    void Unbind(const ComPtr<ID3D11DeviceContext3>& context, BindCallback func) const;

    void UnbindUAV(const ComPtr<ID3D11DeviceContext3>& context) const;

    struct Enumerator
    {
        const D3D11DescriptorRange* descriptorRange;
        BindingValuePair current;
        D3D11ShaderParameter* currentBucket;

        Enumerator(const D3D11DescriptorRange& descriptorRange)
            : descriptorRange(&descriptorRange), current{}, currentBucket(nullptr)
        {
        }

        BindingValuePair& Current()
        {
            return current;
        }

        const BindingValuePair& Current() const
        {
            return current;
        }

        bool MoveNext()
        {
            if (currentBucket == nullptr)
            {
                currentBucket = descriptorRange->buckets;
                if (currentBucket == nullptr) 
                    return false;
                ReadFromBucket();
                return true;
            }

            auto index = currentBucket - descriptorRange->buckets;
            if (index == descriptorRange->bucketCount - 1)
            {
                return false;
            }
            currentBucket++;
            ReadFromBucket();
            return true;
        }

    	void ReadFromBucket()
        {
            current.name = currentBucket->name;
            current.stage = descriptorRange->stage;
            current.type = currentBucket->type;
            current.value = descriptorRange->resources[currentBucket->index];
        }

    	void Reset()
        {
            currentBucket = nullptr;
        }
    };

    Enumerator GetEnumerator() const
    {
        return Enumerator(*this);
    }
};

HEXA_PRISM_NAMESPACE_END