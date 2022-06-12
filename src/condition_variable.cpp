#include <crasy/condition_variable.hpp>

namespace crasy {

condition_variable_wait_future_base::condition_variable_wait_future_base(
    condition_variable& cv)
    : cv_(&cv) {}

void condition_variable_wait_future_base::notify() {
    notified_ = true;
    auto suspended = suspended_;
    suspended_ = std::coroutine_handle<>();
    detail::schedule_task(suspended);
}

void condition_variable::notify_one() {
    auto waiter = waiters_.pop();
    if (waiter.has_value()) { waiter->notify(); }
}

void condition_variable::notify_all() {
    for (;;) {
        auto waiter = waiters_.pop();
        if (waiter.has_value()) {
            waiter->notify();
        } else {
            break;
        }
    }
}

} // namespace crasy
