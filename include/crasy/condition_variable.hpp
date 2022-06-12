#ifndef CRASY_CONDITION_VARIABLE_HPP
#define CRASY_CONDITION_VARIABLE_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <coroutine>
#include <crasy/future.hpp>
#include <crasy/lfqueue.hpp>

namespace crasy {

class condition_variable;

template <typename Lockable>
class condition_variable_wait_future;

class CRASY_API condition_variable_wait_future_base {
  private:
    explicit condition_variable_wait_future_base(condition_variable& cv);

    void notify();

    condition_variable* cv_;
    std::coroutine_handle<> suspended_;
    bool notified_{false};

    template <typename>
    friend class condition_variable_wait_future;

    friend class condition_variable;
};

template <typename Lockable>
class condition_variable_wait_future
    : public condition_variable_wait_future_base {
  public:
    bool await_ready();
    void await_suspend(std::coroutine_handle<> suspended);
    void await_resume();

  private:
    explicit condition_variable_wait_future(condition_variable& cv,
                                            Lockable& lk)
        : condition_variable_wait_future_base(cv), lk_(&lk) {}

    using awaiter = decltype(std::declval<Lockable*>().lock());

    Lockable* lk_;
    bool locked_{false};
    option<awaiter> awaiter_;
};

/// @ingroup sync_grp
class CRASY_API condition_variable {
  public:
    condition_variable() = default;
    condition_variable(const condition_variable&) = delete;
    condition_variable(condition_variable&&) = delete;
    ~condition_variable() = default;
    condition_variable& operator=(const condition_variable&) = delete;
    condition_variable& operator=(condition_variable&&) = delete;

    template <typename Lockable>
    condition_variable_wait_future<Lockable> wait(Lockable& lk) {
        return condition_variable_wait_future<Lockable>(lk);
    }

    template <typename Lockable, typename Pred>
    future<void> wait(Lockable& lk, Pred pred) {
        while (!pred()) { co_await wait(lk); }
    }

    void notify_one();

    void notify_all();

  private:
    lfqueue<condition_variable_wait_future_base&> waiters_;
};

template <typename Lockable>
bool condition_variable_wait_future<Lockable>::await_ready() {
    if (notified_) {
        if (!locked_) {
            locked_ = true;
            awaiter_.emplace(lk_->lock());
        }
        return awaiter_->await_ready();
    } else {
        return false;
    }
}

template <typename Lockable>
void condition_variable_wait_future<Lockable>::await_suspend(
    std::coroutine_handle<> suspended) {
    if (awaiter_.has_value()) {
        awaiter_->await_suspend(suspended);
    } else {
        suspended_ = suspended;
        cv_->waiters_.push(*this);
    }
}

template <typename Lockable>
void condition_variable_wait_future<Lockable>::await_resume() {
    awaiter_->await_resume();
}

} // namespace crasy

#endif
