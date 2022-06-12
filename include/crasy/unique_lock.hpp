#ifndef CRASY_UNIQUE_LOCK_HPP
#define CRASY_UNIQUE_LOCK_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <crasy/future.hpp>

namespace crasy {

template <typename Mutex>
class unique_lock;

template <typename Mutex>
future<unique_lock<Mutex>> lock_unique(Mutex& mtx);

template <typename Mutex>
class unique_lock {
  public:
    unique_lock(const unique_lock&) = delete;

    unique_lock(unique_lock&& other)
        : mtx_(other.mtx_), locked_(other.locked_) {
        other.mtx_ = nullptr;
    }

    ~unique_lock() {
        if (mtx_ != nullptr && locked_) { mtx_->unlock(); }
    }

    unique_lock& operator=(const unique_lock&) = delete;

    unique_lock& operator=(unique_lock&& rhs) {
        std::swap(mtx_, rhs.mtx_);
        std::swap(locked_, rhs.locked_);
        return *this;
    }

    bool try_lock() { return (locked_ = mtx_->try_lock()); }

    auto lock() {
        locked_ = true;
        return mtx_->lock();
    }

    void unlock() {
        locked_ = false;
        return mtx_->unlock();
    }

  private:
    explicit unique_lock(Mutex& mtx) : mtx_(&mtx) {}

    Mutex* mtx_;
    bool locked_{false};

    template <typename M>
    friend future<unique_lock<M>> lock_unique(M&);
};

template <typename Mutex>
future<unique_lock<Mutex>> lock_unique(Mutex& mtx) {
    co_await mtx.lock();
    co_return unique_lock<Mutex>{mtx};
}

} // namespace crasy

#endif
