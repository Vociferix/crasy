#include <crasy/mutex.hpp>

namespace crasy {

bool mutex_lock_future::await_ready() { return mtx_->try_lock(); }

void mutex_lock_future::await_suspend(std::coroutine_handle<> suspended) {
    mtx_->suspended_.push(suspended);
    if (!mtx_->locked_.load(std::memory_order_relaxed)) {
        detail::schedule_task(*mtx_->suspended_.pop());
    }
}

void await_resume() {}

bool mutex::try_lock() { return !locked_.exchange(true); }

void mutex::unlock() {
    locked_.store(false);
    auto suspended = suspended_.pop();
    if (suspended.has_value()) { detail::schedule_task(*suspended); }
}

} // namespace crasy
