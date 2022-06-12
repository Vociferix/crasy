#ifndef CRASY_LOCK_GUARD_HPP
#define CRASY_LOCK_GUARD_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <crasy/future.hpp>

namespace crasy {

template <typename Mutex>
class lock_guard;

template <typename Mutex>
future<lock_guard<Mutex>> lock(Mutex& mtx);

template <typename Mutex>
class lock_guard {
  public:
    lock_guard(const lock_guard&) = delete;

    lock_guard(lock_guard&& other) : mtx_(other.mtx_) { other.mtx_ = nullptr; }

    ~lock_guard() {
        if (mtx_ != nullptr) { mtx_->unlock(); }
    }

    lock_guard& operator=(const lock_guard&) = delete;

    lock_guard& operator=(lock_guard&& rhs) {
        std::swap(mtx_, rhs.mtx_);
        return *this;
    }

  private:
    explicit lock_guard(Mutex& mtx) : mtx_(&mtx) {}

    Mutex* mtx_;

    template <typename M>
    friend future<lock_guard<M>> lock(M&);
};

template <typename Mutex>
future<lock_guard<Mutex>> lock(Mutex& mtx) {
    co_await mtx.lock();
    co_return lock_guard<Mutex>{mtx};
}

namespace detail {

template <typename F, typename M1, typename... MN>
auto lock_rotate_and_call(F&& func, std::size_t count, M1& mtx1, MN&... mtxn) {
    if (count == 0) { return func(mtx1, mtxn...); }
    return lock_rotate_and_call(std::forward<F>(func), count - 1, mtxn...,
                                mtx1);
}

constexpr std::size_t try_lock_impl() { return 0; }

template <typename M1, typename... MN>
std::size_t try_lock_impl(M1& mtx1, MN&... mtxn) {
    if (mtx1.try_lock()) {
        auto cnt = try_lock_impl(mtxn...);
        if (cnt != sizeof...(MN)) { mtx1.unlock(); }
        return 1 + cnt;
    } else {
        return 0;
    }
}

template <typename M1, typename... MN>
future<std::size_t> lock_impl(std::size_t start, M1& mtx1, MN&... mtxn) {
    return lock_rotate_and_call(
        [start](auto& m1, auto&... mn) -> future<std::size_t> {
            co_await m1.lock();
            auto ret = try_lock_impl(mn...);
            if (ret == sizeof...(mn)) {
                co_return ret + 1;
            } else {
                m1.unlock();
                co_return(start + ret + 1) % (sizeof...(MN) + 1);
            }
        },
        start, mtx1, mtxn...);
}

} // namespace detail

template <typename M1, typename M2, typename... MN>
future<std::tuple<lock_guard<M1>, lock_guard<M2>, lock_guard<MN>...>>
lock(M1& mtx1, M2& mtx2, MN&... mtxn) {
    static constexpr std::size_t SUCCESS = sizeof...(MN) + 2;
    std::size_t i = 0;
    do {
        i = co_await detail::lock_impl(i, mtx1, mtx2, mtxn...);
    } while (i != SUCCESS);
    co_return std::make_tuple(lock_guard<M1>{mtx1}, lock_guard<M2>{mtx2},
                              lock_guard<MN>{mtxn}...);
}

} // namespace crasy

#endif
