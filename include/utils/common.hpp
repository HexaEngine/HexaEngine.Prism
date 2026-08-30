#ifndef HEXA_UTILS_COMMON_HPP
#define HEXA_UTILS_COMMON_HPP

#include "config.hpp"
#include <vector>
#include <stack>
#include <string>
#include <memory>
#include <iostream>
#include <functional>
#include <cstdint>
#include <unordered_set>
#include <unordered_map>
#include <stdexcept>
#include <future>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <type_traits>
#include <queue>
#include <condition_variable>
#include <coroutine>
#include <atomic>
#include <assert.h>
#include <algorithm>
#include <chrono>
#include <concepts>
#include <bit>
#include <cstdlib>
#include <barrier>
#include <filesystem>
#include <optional>
#include <array>
#include <semaphore>
#include <bitset>
#include <numeric>
#include <cstring>
#include <fstream>
#include <cmath>

#if !UTILS_NO_GLOBAL_SMART_POINTERS

template <typename T>
using UPtr = std::unique_ptr<T>;

template <typename T, typename... Args>
[[nodiscard]] UPtr<T> make_uptr(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using SPtr = std::shared_ptr<T>;

template <typename T>
using WPtr = std::weak_ptr<T>;

template <typename T, typename... Args>
[[nodiscard]] SPtr<T> make_sptr(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

#endif

namespace HEXA_UTILS_NAMESPACE
{
    template <typename T>
    struct sizeof_t
    {
        static constexpr size_t value = sizeof(T);
    };
    template <>
    struct sizeof_t<void>
    {
        static constexpr size_t value = 0;
    };

    template <typename T>
    static constexpr size_t sizeof_t_v = sizeof_t<T>::value;

    template <typename T>
    struct alignof_t
    {
        static constexpr size_t value = alignof(T);
    };
    template <>
    struct alignof_t<void>
    {
        static constexpr size_t value = 0;
    };

    template <typename T>
    static constexpr size_t alignof_t_v = alignof_t<T>::value;

    template <typename T>
    using clean_type = std::remove_cvref<std::remove_pointer_t<std::remove_reference_t<T>>>;

    template <typename T>
    using clean_type_t = typename clean_type<T>::type;

    template <bool Test, typename T>
    using add_const_cond = std::conditional<Test, std::add_const_t<T>, T>;

    template <bool Test, typename T>
    using add_const_cond_t = typename add_const_cond<Test, T>::type;

    template <bool Test, typename T>
    using add_pointer_cond = std::conditional<Test, std::add_pointer_t<T>, T>;

    template <bool Test, typename T>
    using add_pointer_cond_t = typename add_pointer_cond<Test, T>::type;

    template <bool Test, typename T>
    using add_lvalue_reference_cond = std::conditional<Test, std::add_lvalue_reference_t<T>, T>;

    template <bool Test, typename T>
    using add_lvalue_reference_cond_t = typename add_lvalue_reference_cond<Test, T>::type;

    template <bool Test, typename T>
    using add_rvalue_reference_cond = std::conditional<Test, std::add_rvalue_reference_t<T>, T>;

    template <bool Test, typename T>
    using add_rvalue_reference_cond_t = typename add_rvalue_reference_cond<Test, T>::type;

    template <bool Test, typename T>
    using add_cv_cond = std::conditional<Test, std::add_cv_t<T>, T>;

    template <bool Test, typename T>
    using add_cv_cond_t = typename add_cv_cond<Test, T>::type;

    template <bool Test, typename T>
    using add_volatile_cond = std::conditional<Test, std::add_volatile_t<T>, T>;

    template <bool Test, typename T>
    using add_volatile_cond_t = typename add_volatile_cond<Test, T>::type;

    static constexpr size_t NextPowerOfTwo(size_t n)
    {
        if (n == 0)
            return 1;
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        if constexpr (sizeof(size_t) == 8)
            n |= n >> 32;
        return n + 1;
    }
} // namespace HEXA_UTILS_NAMESPACE

#endif // UTILS_COMMON_HPP