#ifndef CRASY_INTERNAL_HPP
#define CRASY_INTERNAL_HPP

#include <array>
#include <type_traits>
#include <utility>

#include <asio/buffer.hpp>

namespace crasy {

template <typename T>
asio::const_buffer asio_buffer(std::span<const T> buf) {
    return asio::const_buffer(buf.data(), buf.size() * sizeof(T));
}

template <typename T>
asio::mutable_buffer asio_buffer(std::span<T> buf) {
    return asio::mutable_buffer(buf.data(), buf.size() * sizeof(T));
}

} // namespace crasy

#endif
