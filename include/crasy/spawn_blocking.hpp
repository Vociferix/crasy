#ifndef CRASY_SPAWN_BLOCKING_HPP
#define CRASY_SPAWN_BLOCKING_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <crasy/detail.hpp>
#include <crasy/future.hpp>

#include <mutex>

namespace crasy {

template <typename T>
class blocking_join_handle;

template <typename F>
blocking_join_handle<decltype(std::declval<F>()())> spawn_blocking(F&& func);

template <typename T>
class blocking_join_handle {
  public:
    blocking_join_handle() = default;

    blocking_join_handle(const blocking_join_handle&) = delete;

    blocking_join_handle(blocking_join_handle&& other) : state_(other.state_) {
        other.state_ = nullptr;
    }

    ~blocking_join_handle() {
        if (state_ != nullptr) { detach(); }
    }

    blocking_join_handle& operator=(const blocking_join_handle&) = delete;

    blocking_join_handle& operator=(blocking_join_handle&& rhs) {
        std::swap(state_, rhs.state_);
        return *this;
    }

    bool await_ready() {
        std::lock_guard<std::mutex> lock{state_->mtx};
        return state_->state == 1;
    }

    void await_suspend(std::coroutine_handle<> suspended) {
        std::lock_guard<std::mutex> lock{state_->mtx};
        if (state_->state == 1) {
            detail::schedule_task(suspended);
        } else {
            state_->suspended = suspended;
        }
    }

    T await_resume() {
        std::lock_guard<std::mutex> lock{state_->mtx};
        return *std::move(state_->ret);
    }

    void detach() {
        std::unique_lock<std::mutex> lock{state_->mtx};
        if (state_->state == 1) {
            lock.unlock();
            lock.release();
            delete state_;
        } else {
            state_->state = -1;
        }
        state_ = nullptr;
    }

    operator bool() const { return state_ != nullptr; }

  private:
    struct state_base {
        state_base() = default;
        state_base(const state_base&) = delete;
        state_base(state_base&&) = delete;
        virtual ~state_base() = default;
        state_base& operator=(const state_base&) = delete;
        state_base& operator=(state_base&&) = delete;

        virtual void call() = 0;

        std::mutex mtx;
        option<T> ret;
        std::exception_ptr ex{nullptr};
        std::coroutine_handle<> suspended;
        char state{0};
    };

    template <typename F>
    struct state_t : state_base {
        F func;

        state_t(F&& f) : func(std::forward<F>(f)) {}
        state_t(const state_t&) = delete;
        state_t(state_t&&) = delete;
        ~state_t() override = default;
        state_t& operator=(const state_t&) = delete;
        state_t& operator=(state_t&&) = delete;

        void call() override {
            std::coroutine_handle<> susp;
            char st = 0;
            try {
                auto&& tmp = func();
                std::lock_guard<std::mutex> lock{this->mtx};
                this->ret.emplace(std::forward<T>(tmp));
                susp = this->suspended;
                this->suspended = std::coroutine_handle<>();
                st = this->state;
                this->state = 1;
            } catch (...) {
                std::lock_guard<std::mutex> lock{this->mtx};
                this->ex = std::current_exception();
                susp = this->suspended;
                this->suspended = std::coroutine_handle();
                st = this->state;
                this->state = 1;
            }
            if (susp) { detail::schedule_task(susp); }
            if (st == -1) { delete this; }
        }
    };

    static void run(void* state) {
        reinterpret_cast<state_base*>(state)->call();
    }

    explicit blocking_join_handle(state_base& state) : state_(&state) {
        detail::run_blocking(&run, state_);
    }

    state_base* state_{nullptr};

    template <typename F>
    friend blocking_join_handle<decltype(std::declval<F>()())> spawn_blocking(
        F&&);
};

template <>
class blocking_join_handle<void> {
  public:
    blocking_join_handle() = default;

    blocking_join_handle(const blocking_join_handle&) = delete;

    blocking_join_handle(blocking_join_handle&& other) : state_(other.state_) {
        other.state_ = nullptr;
    }

    ~blocking_join_handle() {
        if (state_ != nullptr) { detach(); }
    }

    blocking_join_handle& operator=(const blocking_join_handle&) = delete;

    blocking_join_handle& operator=(blocking_join_handle&& rhs) {
        std::swap(state_, rhs.state_);
        return *this;
    }

    bool await_ready() {
        std::lock_guard<std::mutex> lock{state_->mtx};
        return state_->state == 1;
    }

    void await_suspend(std::coroutine_handle<> suspended) {
        std::lock_guard<std::mutex> lock{state_->mtx};
        if (state_->state == 1) {
            detail::schedule_task(suspended);
        } else {
            state_->suspended = suspended;
        }
    }

    void await_resume() {}

    void detach() {
        std::unique_lock<std::mutex> lock{state_->mtx};
        if (state_->state == 1) {
            lock.unlock();
            lock.release();
            delete state_;
        } else {
            state_->state = -1;
        }
        state_ = nullptr;
    }

    operator bool() const { return state_ != nullptr; }

  private:
    struct state_base {
        state_base() = default;
        state_base(const state_base&) = delete;
        state_base(state_base&&) = delete;
        virtual ~state_base() = default;
        state_base& operator=(const state_base&) = delete;
        state_base& operator=(state_base&&) = delete;

        virtual void call() = 0;

        std::mutex mtx;
        std::exception_ptr ex{nullptr};
        std::coroutine_handle<> suspended;
        char state{0};
    };

    template <typename F>
    struct state_t : state_base {
        F func;

        state_t(F&& f) : func(std::forward<F>(f)) {}
        state_t(const state_t&) = delete;
        state_t(state_t&&) = delete;
        ~state_t() override = default;
        state_t& operator=(const state_t&) = delete;
        state_t& operator=(state_t&&) = delete;

        void call() override {
            std::coroutine_handle<> susp;
            char st = 0;
            try {
                func();
                std::lock_guard<std::mutex> lock{this->mtx};
                susp = this->suspended;
                this->suspended = std::coroutine_handle<>();
                st = this->state;
                this->state = 1;
            } catch (...) {
                std::lock_guard<std::mutex> lock{this->mtx};
                this->ex = std::current_exception();
                susp = this->suspended;
                this->suspended = std::coroutine_handle();
                st = this->state;
                this->state = 1;
            }
            if (susp) { detail::schedule_task(susp); }
            if (st == -1) { delete this; }
        }
    };

    static void run(void* state) {
        reinterpret_cast<state_base*>(state)->call();
    }

    explicit blocking_join_handle(state_base& state) : state_(&state) {
        detail::run_blocking(&run, state_);
    }

    state_base* state_{nullptr};

    template <typename F>
    friend blocking_join_handle<decltype(std::declval<F>()())> spawn_blocking(
        F&&);
};

template <typename F>
blocking_join_handle<decltype(std::declval<F>()())> spawn_blocking(F&& func) {
    using ret_t = decltype(func());
    using handle_t = blocking_join_handle<ret_t>;
    using state_t = typename handle_t::state_t<F>;
    return handle_t{*new state_t(std::forward<F>(func))};
}

} // namespace crasy

#endif
