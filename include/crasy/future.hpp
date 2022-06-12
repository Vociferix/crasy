#ifndef CRASY_FUTURE_HPP
#define CRASY_FUTURE_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <coroutine>
#include <exception>

#include <crasy/detail.hpp>
#include <crasy/option.hpp>

namespace crasy {

template <typename T>
class future {
  public:
    using return_type = T;

    class promise_type {
      public:
        future get_return_object() {
            return future(
                std::coroutine_handle<promise_type>::from_promise(*this));
        }

        void unhandled_exception() { ex_ = std::current_exception(); }

        std::suspend_never initial_suspend() { return {}; }

        std::suspend_always final_suspend() noexcept {
            done_ = true;
            if (suspended_) {
                auto suspended = suspended_;
                suspended_ = std::coroutine_handle<>();
                detail::schedule_task(suspended);
            }
            return {};
        }

        void return_value(T&& value) { value_.emplace(std::forward<T>(value)); }

      private:
        option<T> value_;
        std::exception_ptr ex_{nullptr};
        std::coroutine_handle<> suspended_;
        bool done_{false};

        friend class future;
    };

    explicit future(std::coroutine_handle<promise_type> handle)
        : handle_(handle) {}

    future(const future&) = delete;

    future(future&& other) : handle_(other.handle_) {
        other.handle_ = std::coroutine_handle<promise_type>();
    }

    ~future() {
        if (handle_) { handle_.destroy(); }
    }

    future& operator=(const future&) = delete;

    future& operator=(future&& rhs) noexcept {
        auto tmp = handle_;
        handle_ = rhs.handle_;
        rhs.handle_ = tmp;
        return *this;
    }

    bool await_ready() const { return handle_.promise().done_; }

    void await_suspend(std::coroutine_handle<> suspended) const {
        handle_.promise().suspended_ = suspended;
    }

    T await_resume() const {
        auto& promise = handle_.promise();
        if (promise.ex_ != nullptr) {
            auto ex = promise.ex_;
            promise.ex_ = nullptr;
            std::rethrow_exception(ex);
        }
        return *std::move(promise.value_);
    }

    std::coroutine_handle<promise_type> into_handle() && {
        auto handle = handle_;
        handle_ = std::coroutine_handle<promise_type>();
        return handle;
    }

  private:
    std::coroutine_handle<promise_type> handle_;
};

template <>
class future<void> {
  public:
    class promise_type {
      public:
        future get_return_object() {
            return future(
                std::coroutine_handle<promise_type>::from_promise(*this));
        }

        void unhandled_exception() { ex_ = std::current_exception(); }

        std::suspend_never initial_suspend() { return {}; }

        std::suspend_always final_suspend() noexcept {
            done_ = true;
            if (suspended_) {
                auto suspended = suspended_;
                suspended_ = std::coroutine_handle<>();
                detail::schedule_task(suspended);
            }
            return {};
        }

        void return_void() {}

      private:
        std::exception_ptr ex_{nullptr};
        std::coroutine_handle<> suspended_;
        bool done_{false};

        friend class future;
    };

    explicit future(std::coroutine_handle<promise_type> handle)
        : handle_(handle) {}

    future(const future&) = delete;

    future(future&& other) : handle_(other.handle_) {
        other.handle_ = std::coroutine_handle<promise_type>();
    }

    ~future() {
        if (handle_) { handle_.destroy(); }
    }

    future& operator=(const future&) = delete;

    future& operator=(future&& rhs) noexcept {
        auto tmp = handle_;
        handle_ = rhs.handle_;
        rhs.handle_ = tmp;
        return *this;
    }

    bool await_ready() const { return handle_.promise().done_; }

    void await_suspend(std::coroutine_handle<> suspended) const {
        handle_.promise().suspended_ = suspended;
    }

    void await_resume() const {
        auto& promise = handle_.promise();
        if (promise.ex_ != nullptr) {
            auto ex = promise.ex_;
            promise.ex_ = nullptr;
            std::rethrow_exception(ex);
        }
    }

    std::coroutine_handle<promise_type> into_handle() && {
        auto handle = handle_;
        handle_ = std::coroutine_handle<promise_type>();
        return handle;
    }

  private:
    std::coroutine_handle<promise_type> handle_;
};

} // namespace crasy

#endif
