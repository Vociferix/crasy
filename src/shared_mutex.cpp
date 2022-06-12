#include <crasy/shared_mutex.hpp>

namespace crasy {

inline constexpr auto EX_BIT = ~(~std::size_t{0} >> 1);

bool shared_mutex_lock_future::await_ready() {
    if (requested_) {
        return mtx_->state_.load() == EX_BIT;
    } else {
        auto state = mtx_->state_.load();
        while ((state & EX_BIT) == 0) {
            if (mtx_->state_.compare_exchange_strong(state, state | EX_BIT)) {
                requested_ = true;
                return state == EX_BIT;
            }
        }
        return false;
    }
}

void shared_mutex_lock_future::await_suspend(
    std::coroutine_handle<> suspended) {
    mtx_->suspended_.push(suspended);
    if (mtx_->state_.load(std::memory_order_relaxed) == 0) {
        for (;;) {
            auto sus = mtx_->suspended_.pop();
            if (sus.has_value()) {
                detail::schedule_task(*sus);
            } else {
                break;
            }
        }
    }
}

void shared_mutex_lock_future::await_resume() {}

bool shared_mutex_lock_shared_future::await_ready() { return mtx_->try_lock(); }

void shared_mutex_lock_shared_future::await_suspend(
    std::coroutine_handle<> suspended) {
    mtx_->suspended_.push(suspended);
    if ((mtx_->state_.load(std::memory_order_relaxed) & EX_BIT) == 0) {
        for (;;) {
            auto sus = mtx_->suspended_.pop();
            if (sus.has_value()) {
                detail::schedule_task(*sus);
            } else {
                break;
            }
        }
    }
}

void shared_mutex_lock_shared_future::await_resume() {}

bool shared_mutex::try_lock() {
    std::size_t expected = 0;
    return state_.compare_exchange_strong(expected, EX_BIT);
}

void shared_mutex::unlock() {
    state_.fetch_sub(EX_BIT);
    for (;;) {
        auto suspended = suspended_.pop();
        if (suspended.has_value()) {
            detail::schedule_task(*suspended);
        } else {
            break;
        }
    }
}

bool shared_mutex::try_lock_shared() {
    auto tmp = state_.fetch_add(1);
    if ((tmp & EX_BIT) == 0) {
        return true;
    } else {
        state_.fetch_sub(1);
        return false;
    }
}

void shared_mutex::unlock_shared() {
    if ((state_.fetch_sub(1) & ~EX_BIT) == 1) {
        for (;;) {
            auto suspended = suspended_.pop();
            if (suspended.has_value()) {
                detail::schedule_task(*suspended);
            } else {
                break;
            }
        }
    }
}

} // namespace crasy
