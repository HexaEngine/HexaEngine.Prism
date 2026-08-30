#ifndef HEXA_UTILS_CONFIG_HPP
#define HEXA_UTILS_CONFIG_HPP

#if defined(_WIN32) || defined(_WIN64)
#define HEXA_UTILS_WINDOWS 1
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

extern bool UtilsEnableAsserts;

#define HEXA_UTILS_ASSERT(expr, message) if (!(expr) && UtilsEnableAsserts) { fprintf(stderr, "ASSERTION FAILED: %s\n", message); assert(false && message); }
#define HEXA_UTILS_DEBUG 1
#endif

#ifdef _MSC_VER
#define HEXA_ASSUME_ALIGNED(ptr, align) __assume(((uintptr_t)(ptr) & ((align)-1)) == 0)
#elif defined(__GNUC__) || defined(__clang__)
#define HEXA_ASSUME_ALIGNED(ptr, align) ptr = (decltype(ptr))__builtin_assume_aligned(ptr, align)
#else
#define HEXA_ASSUME_ALIGNED(ptr, align) ((void)0)
#endif

#ifndef HEXA_UTILS_NAMESPACE
#define HEXA_UTILS_NAMESPACE Utils
#endif

#if (defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)) && !defined(_M_ARM64EC)
#define HEXA_UTILS_X86_CPU_TARGET 1
#if defined (_MSC_VER)
#define HEXA_UTILS_SVML 1
#endif
#else
#define HEXA_UTILS_X86_CPU_TARGET 0
#define HEXA_UTILS_SVML 0
#endif

#ifndef HEXA_UTILS_SVML
#define HEXA_UTILS_SVML 0
#endif

#if defined(__aarch64__) || defined(_M_ARM64) || defined(__arm__) || defined(_M_ARM) || defined(_M_ARM64EC)
#define HEXA_UTILS_ARM_CPU_TARGET 1
#else
#define HEXA_UTILS_ARM_CPU_TARGET 0
#endif