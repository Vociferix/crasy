#ifndef CRASY_DETAIL_HPP
#define CRASY_DETAIL_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <asio/io_context.hpp>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include <coroutine>

namespace crasy::detail {

CRASY_API bool in_executor_context();
CRASY_API void schedule_task(std::coroutine_handle<> handle);
CRASY_API asio::io_context& context();
CRASY_API void run_blocking(void (*func)(void*), void* data);

template <typename U>
struct remove_rvalue_reference {
    using type = U;
};

template <typename U>
struct remove_rvalue_reference<U&&> {
    using type = U;
};

template <typename U>
using remove_rvalue_reference_t = typename remove_rvalue_reference<U>::type;

template <typename T>
struct type_kinds {
    using value_type = T;
    using reference = T&;
    using const_reference = std::add_const_t<T>&;
    using rvalue_reference = T&&;
    using const_rvalue_reference = std::add_const_t<T>&&;
    using pointer = T*;
    using const_pointer = std::add_const_t<T>*;
};

template <typename T>
struct type_kinds<T&> {
    using value_type = T&;
    using reference = T&;
    using const_reference = std::add_const_t<T>&;
    using rvalue_reference = T&;
    using const_rvalue_reference = std::add_const_t<T>&;
    using pointer = T*;
    using const_pointer = std::add_const_t<T>*;
};

template <typename T>
struct type_kinds<T&&> {
    using value_type = T;
    using reference = T&;
    using const_reference = std::add_const_t<T>&;
    using rvalue_reference = T&&;
    using const_rvalue_reference = std::add_const_t<T>&&;
    using pointer = T*;
    using const_pointer = std::add_const_t<T>*;
};

template <>
struct type_kinds<void> {
    using value_type = void;
    using reference = void;
    using const_reference = void;
    using rvalue_reference = void;
    using const_rvalue_reference = void;
    using pointer = void;
    using const_pointer = void;
};

} // namespace crasy::detail

#endif
