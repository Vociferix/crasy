#include <crasy/detail.hpp>
#include <crasy/io_future.hpp>

namespace crasy::detail {

#define FUTURE_DONE (reinterpret_cast<void*>(1))

bool io_future::await_ready() const { return suspended_.load() != nullptr; }

void io_future::await_suspend(std::coroutine_handle<> suspended) {
    auto handle = suspended.address();
    if (suspended_.exchange(handle) == FUTURE_DONE) {
        detail::schedule_task(suspended);
    }
}

void io_future::finish() {
    auto handle = suspended_.exchange(FUTURE_DONE);
    if (handle != nullptr) {
        detail::schedule_task(std::coroutine_handle<>::from_address(handle));
    }
}

} // namespace crasy::detail
