#ifndef CRASY_UTILS_HPP
#define CRASY_UTILS_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <crasy/result.hpp>

#include <span>

namespace crasy {

CRASY_API result<std::size_t> available_cpu_cores();

template <typename T>
concept byte_like = sizeof(T) == 1 && std::is_trivially_copyable_v<T>;

template <typename T>
concept byte_like_pointer =
    std::is_pointer_v<T> && byte_like<std::remove_pointer_t<T>>;

template <typename T>
concept buffer = requires(T& buf) {
    { std::as_bytes(std::span(buf)) } -> std::same_as<std::span<std::byte>>;
}
&&requires(const T& buf) {
    {
        std::as_bytes(std::span(buf))
        } -> std::convertible_to<std::span<const std::byte>>;
};

} // namespace crasy

#endif
