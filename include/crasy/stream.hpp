#ifndef CRASY_STREAM_HPP
#define CRASY_STREAM_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <crasy/future.hpp>

namespace crasy {

template <typename T>
class stream {
  public:
    using yield_type = T;

    class promise_type {
      public:
        stream get_return_object() {
            return stream(
                std::coroutine_handle<promise_type>::from_promise(*this));
        }

        void unhandled_exception() { ex_ = std::current_exception(); }

        std::suspend_never initial_suspend() { return {}; }

        std::suspend_always final_suspend() {
            done_ = true;
            if (suspended_) {
                auto suspended = suspended_;
                suspended_ = std::coroutine_handle<>();
                detail::schedule_task(suspended);
            }
            return {};
        }

        void yield_value(T&& value) {
            value_.emplace(std::forward<T>(value));
            if (suspended_) {
                auto suspended = suspended_;
                suspended_ = std::coroutine_handle<>();
                detail::schedule_task(suspended);
            }
        }

        void return_void() {}

      private:
        option<T> value_;
        std::exception_ptr ex_{nullptr};
        std::coroutine_handle<> suspended_;
        bool done_{false};

        template <typename>
        friend class future;
    };

    explicit stream(std::coroutine_handle<promise_type> handle)
        : handle_(handle) {}

    stream(const stream&) = delete;

    stream(stream&& other) : handle_(other.handle_) {
        other.handle_ = std::coroutine_handle<promise_type>();
    }

    ~stream() {
        if (handle_) { handle_.destroy(); }
    }

    stream& operator=(const stream&) = delete;

    stream& operator=(stream&& rhs) {
        auto tmp = handle_;
        handle_ = rhs.handle_;
        rhs.handle_ = tmp;
        return *this;
    }

    bool await_ready() {
        auto& promise = handle_.promise();
        return promise.done_ || promise.value_.has_value() ||
               promise.ex_ != nullptr;
    }

    void await_suspend(std::coroutine_handle<> handle) {
        handle_.promise().suspended_ = handle;
    }

    option<T> await_resume() {
        auto& promise = handle_.promise();
        if (promise.ex_ != nullptr) {
            auto ex = promise.ex_;
            promise.ex_ = nullptr;
            std::rethrow_exception(ex);
        }
        return std::move(promise.value_);
    }

    template <typename F>
    future<void> foreach (F&& func) {
        static constexpr bool is_cr = requires(F && f, option<T> v) {
            co_await f(*std::move(v));
        };

        for (;;) {
            auto val = co_await *this;
            if (val.has_value()) {
                if constexpr (is_cr) {
                    co_await f(*std::move(val));
                } else {
                    f(*std::move(val));
                }
            } else {
                break;
            }
        }
    }

    std::coroutine_handle<promise_type> into_handle() && {
        auto ret = handle_;
        handle_ = std::coroutine_handle<promise_type>();
        return ret;
    }

  private:
    std::coroutine_handle<promise_type> handle_;
};

} // namespace crasy

#endif
