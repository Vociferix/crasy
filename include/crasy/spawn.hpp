#ifndef CRASY_SPAWN_HPP
#define CRASY_SPAWN_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <crasy/detail.hpp>
#include <crasy/future.hpp>

#include <mutex>

namespace crasy {

namespace detail {

template <typename T>
class join_handle_impl {
  public:
    class promise_type {
      public:
        join_handle_impl get_return_object() {
            return join_handle_impl{
                std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        void unhandled_exception() {
            std::lock_guard<std::mutex> lock{mtx_};
            ex_ = std::current_exception();
        }

        std::suspend_never initial_suspend() { return {}; }

        std::suspend_always final_suspend() {
            std::unique_lock<std::mutex> lock{mtx_};
            if (state_ == detached) {
                lock.unlock();
                lock.release();
                std::coroutine_handle<promise_type>::from_promise(*this)
                    .destroy();
            } else {
                state_ = done;
                if (suspended_) {
                    auto suspended = suspended_;
                    suspended_ = std::coroutine_handle<>();
                    detail::schedule_task(suspended);
                }
            }
            return {};
        }

        void return_value(T&& value) {
            std::lock_guard<std::mutex> lock{mtx_};
            value_.emplace(std::forward<T>(value));
        }

      private:
        static inline constexpr char waiting = 0;
        static inline constexpr char done = 1;
        static inline constexpr char detached = -1;

        std::mutex mtx_;
        option<T> value_;
        std::exception_ptr ex_{nullptr};
        std::coroutine_handle<> suspended_;
        char state_{waiting};

        friend class join_handle_impl;
    };

    join_handle_impl() = default;

    join_handle_impl(const join_handle_impl&) = delete;

    join_handle_impl(join_handle_impl&& other) : handle_(other.handle_) {
        other.handle_ = std::coroutine_handle<>();
    }

    ~join_handle_impl() {
        if (handle_) { detach(); }
    }

    join_handle_impl& operator=(const join_handle_impl&) = delete;

    join_handle_impl& operator=(join_handle_impl&& rhs) {
        auto tmp = handle_;
        handle_ = rhs.handle_;
        rhs.handle_ = tmp;
        return *this;
    }

    bool await_ready() const {
        auto& promise = handle_.promise();
        std::lock_guard<std::mutex> lock{promise.mtx_};
        return promise.state_ == promise_type::done;
    }

    void await_suspend(std::coroutine_handle<> suspended) {
        auto& promise = handle_.promise();
        std::lock_guard<std::mutex> lock{promise.mtx_};
        if (promise.state_ == promise_type::waiting) {
            promise.suspended_ = suspended;
        } else {
            detail::schedule_task(suspended);
        }
    }

    T await_resume() {
        auto& promise = handle_.promise();
        std::lock_guard<std::mutex> lock{promise.mtx_};
        if (promise.ex_ != nullptr) {
            auto ex = promise.ex_;
            promise.ex_ = nullptr;
            std::rethrow_exception(ex);
        }
        return *std::move(promise.value_);
    }

    void detach() {
        auto& promise = handle_.promise();
        std::unique_lock<std::mutex> lock{promise.mtx_};
        if (promise.state_ == promise_type::done) {
            lock.unlock();
            lock.release();
            handle_.destroy();
        } else {
            promise.state_ = promise_type::detached;
        }
    }

    operator bool() const { return static_cast<bool>(handle_); }

  private:
    explicit join_handle_impl(std::coroutine_handle<promise_type> handle)
        : handle_(handle) {}

    std::coroutine_handle<promise_type> handle_;

    friend class promise_type;
};

template <>
class join_handle_impl<void> {
  public:
    class promise_type {
      public:
        join_handle_impl get_return_object() {
            return join_handle_impl{
                std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        void unhandled_exception() {
            std::lock_guard<std::mutex> lock{mtx_};
            ex_ = std::current_exception();
        }

        std::suspend_never initial_suspend() { return {}; }

        std::suspend_always final_suspend() noexcept {
            std::unique_lock<std::mutex> lock{mtx_};
            if (state_ == detached) {
                lock.unlock();
                lock.release();
                std::coroutine_handle<promise_type>::from_promise(*this)
                    .destroy();
            } else {
                state_ = done;
                if (suspended_) {
                    auto suspended = suspended_;
                    suspended_ = std::coroutine_handle<>();
                    detail::schedule_task(suspended);
                }
            }
            return {};
        }

        void return_void() {}

      private:
        static inline constexpr char waiting = 0;
        static inline constexpr char done = 1;
        static inline constexpr char detached = -1;

        std::mutex mtx_;
        std::exception_ptr ex_{nullptr};
        std::coroutine_handle<> suspended_;
        char state_{waiting};

        friend class join_handle_impl;
    };

    join_handle_impl() = default;

    join_handle_impl(const join_handle_impl&) = delete;

    join_handle_impl(join_handle_impl&& other) : handle_(other.handle_) {
        other.handle_ = std::coroutine_handle<promise_type>();
    }

    ~join_handle_impl() {
        if (handle_) { detach(); }
    }

    join_handle_impl& operator=(const join_handle_impl&) = delete;

    join_handle_impl& operator=(join_handle_impl&& rhs) {
        auto tmp = handle_;
        handle_ = rhs.handle_;
        rhs.handle_ = tmp;
        return *this;
    }

    bool await_ready() const {
        auto& promise = handle_.promise();
        std::lock_guard<std::mutex> lock{promise.mtx_};
        return promise.state_ == promise_type::done;
    }

    void await_suspend(std::coroutine_handle<> suspended) {
        auto& promise = handle_.promise();
        std::lock_guard<std::mutex> lock{promise.mtx_};
        if (promise.state_ == promise_type::waiting) {
            promise.suspended_ = suspended;
        } else {
            detail::schedule_task(suspended);
        }
    }

    void await_resume() {
        auto& promise = handle_.promise();
        std::lock_guard<std::mutex> lock{promise.mtx_};
        if (promise.ex_ != nullptr) {
            auto ex = promise.ex_;
            promise.ex_ = nullptr;
            std::rethrow_exception(ex);
        }
    }

    void detach() {
        auto& promise = handle_.promise();
        std::unique_lock<std::mutex> lock{promise.mtx_};
        if (promise.state_ == promise_type::done) {
            lock.unlock();
            lock.release();
            handle_.destroy();
        } else {
            promise.state_ = promise_type::detached;
        }
    }

    operator bool() const { return static_cast<bool>(handle_); }

  private:
    explicit join_handle_impl(std::coroutine_handle<promise_type> handle)
        : handle_(handle) {}

    std::coroutine_handle<promise_type> handle_;

    friend class promise_type;
};

} // namespace detail

template <typename T>
class join_handle;

template <typename T>
join_handle<T> spawn(future<T> fut);

template <typename T>
class join_handle {
  public:
    join_handle() = default;
    join_handle(const join_handle&) = delete;
    join_handle(join_handle&&) = default;
    ~join_handle() = default;
    join_handle& operator=(const join_handle&) = delete;
    join_handle& operator=(join_handle&&) = default;

    bool await_ready() const { return handle_.await_ready(); }

    void await_suspend(std::coroutine_handle<> suspended) {
        return handle_.await_suspend(suspended);
    }

    T await_resume() {
        if constexpr (std::is_same_v<T, void>) {
            handle_.await_resume();
        } else {
            return handle_.await_resume();
        }
    }

    void detach() { handle_.detach(); }

    operator bool() const { return static_cast<bool>(handle_); }

  private:
    explicit join_handle(detail::join_handle_impl<T> handle)
        : handle_(std::move(handle)) {}

    detail::join_handle_impl<T> handle_;

    template <typename U>
    friend join_handle<U> spawn(future<U> fut);
};

template <typename T>
join_handle<T> spawn(future<T> fut) {
    return join_handle<T>{[](future<T> f) -> detail::join_handle_impl<T> {
        if constexpr (std::is_same_v<T, void>) {
            co_await f;
            co_return;
        } else {
            co_return co_await f;
        }
    }(std::move(fut))};
}

template <typename F>
auto spawn(F&& func) {
    static constexpr auto is_async = requires(decltype(func()) ret) {
        { ret.await_ready() } -> std::same_as<bool>;
        ret.await_suspend(std::coroutine_handle<>());
        ret.await_resume();
    };
    if constexpr (is_async) { return spawn(func()); }
    else {
        using ret_t = decltype(func());
        return spawn([func = std::forward<F>(func)]() -> future<ret_t> {
            if constexpr (std::is_same_v<ret_t, void>) {
                func();
                co_return;
            } else {
                co_return func();
            }
        }());
    }
}

} // namespace crasy

#endif
