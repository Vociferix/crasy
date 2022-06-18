#ifndef CRASY_UTILS_HPP
#define CRASY_UTILS_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <crasy/result.hpp>

namespace crasy {

CRASY_API result<std::size_t> available_cpu_cores();

template <typename T>
concept byte_like = sizeof(T) == 1 && std::is_trivially_copyable_v<T>;

template <typename T>
concept byte_like_pointer =
    std::is_pointer_v<T> && byte_like<std::remove_pointer_t<T>>;

template <typename T>
concept dynamic_buffer = requires(T buf) {
    { std::size(buf) } -> std::convertible_to<std::size_t>;
    { std::data(buf) } -> byte_like_pointer;
    buf.resize(std::declval<std::size_t>());
};

} // namespace crasy

#endif
