#ifndef CRASY_MUTEX_HPP
#define CRASY_MUTEX_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <atomic>

#include <crasy/future.hpp>
#include <crasy/lfqueue.hpp>

namespace crasy {

class mutex;

class CRASY_API mutex_lock_future {
  public:
    bool await_ready();
    void await_suspend(std::coroutine_handle<> suspended);
    void await_resume();

  private:
    explicit mutex_lock_future(mutex& mtx);

    mutex* mtx_;
};

/// @ingroup sync_grp
class CRASY_API mutex {
  public:
    mutex() = default;
    mutex(const mutex&) = delete;
    mutex(mutex&&) = delete;
    ~mutex() = default;
    mutex& operator=(const mutex&) = delete;
    mutex& operator=(mutex&&) = delete;

    bool try_lock();
    mutex_lock_future lock();
    void unlock();

  private:
    std::atomic<bool> locked_{false};
    lfqueue<std::coroutine_handle<>> suspended_;

    friend class mutex_lock_future;
};

} // namespace crasy

#endif
