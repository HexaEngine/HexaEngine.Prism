#pragma once
#include <atomic>
#include <cstdint>
#include <memory>
#include <type_traits>

#define HEXA_PRISM_NAMESPACE HexaEngine::Prism
#define HEXA_PRISM_NAMESPACE_BEGIN namespace HexaEngine { namespace Prism {
#define HEXA_PRISM_NAMESPACE_END } }

#if defined(_WIN64) || defined(_WIN32)
#define HEXA_PRISM_WINDOWS 1
#endif
