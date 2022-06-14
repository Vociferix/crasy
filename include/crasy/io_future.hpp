#ifndef CRASY_IO_FUTURE_HPP
#define CRASY_IO_FUTURE_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <atomic>
#include <coroutine>

namespace crasy::detail {

class io_future {
  public:
    bool await_ready() const;
    void await_suspend(std::coroutine_handle<> suspended);

  protected:
    void finish();

  private:
    std::atomic<void*> suspended_;
};

} // namespace crasy::detail

#endif
