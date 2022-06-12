#ifndef CRASY_SHARED_MUTEX_HPP
#define CRASY_SHARED_MUTEX_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <atomic>

#include <crasy/future.hpp>
#include <crasy/lfqueue.hpp>

namespace crasy {

class shared_mutex;

class CRASY_API shared_mutex_lock_future {
  public:
    bool await_ready();
    void await_suspend(std::coroutine_handle<> suspended);
    void await_resume();

  private:
    explicit shared_mutex_lock_future(shared_mutex& mtx);

    shared_mutex* mtx_;
    bool requested_{false};
};

class CRASY_API shared_mutex_lock_shared_future {
  public:
    bool await_ready();
    void await_suspend(std::coroutine_handle<> suspended);
    void await_resume();

  private:
    explicit shared_mutex_lock_shared_future(shared_mutex& mtx);
    shared_mutex* mtx_;
};

class CRASY_API shared_mutex {
  public:
    shared_mutex() = default;
    shared_mutex(const shared_mutex&) = delete;
    shared_mutex(shared_mutex&&) = delete;
    ~shared_mutex() = default;
    shared_mutex& operator=(const shared_mutex&) = delete;
    shared_mutex& operator=(shared_mutex&&) = delete;

    bool try_lock();
    shared_mutex_lock_future lock();
    void unlock();

    bool try_lock_shared();
    shared_mutex_lock_shared_future lock_shared();
    void unlock_shared();

  private:
    std::atomic<std::size_t> state_{0};
    lfqueue<std::coroutine_handle<>> suspended_;

    friend class shared_mutex_lock_future;
    friend class shared_mutex_lock_shared_future;
};

} // namespace crasy

#endif
