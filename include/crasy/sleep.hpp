#ifndef CRASY_SLEEP_HPP
#define CRASY_SLEEP_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <asio/basic_waitable_timer.hpp>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include <chrono>

#include <crasy/detail.hpp>

namespace crasy {

template <typename Clock>
class sleep_future;

template <typename Rep, typename Period>
sleep_future<std::chrono::high_resolution_clock> sleep_for(
    const std::chrono::duration<Rep, Period>& timeout);

template <typename Clock, typename Dur>
sleep_future<Clock> sleep_until(
    const std::chrono::time_point<Clock, Dur>& timeout);

template <typename Clock>
class sleep_future {
  public:
    bool await_ready() { return done_; }

    void await_suspend(std::coroutine_handle<> suspended) {
        timer_.async_wait([this, suspended]([[maybe_unused]] auto ec) {
            done_ = true;
            detail::schedule_task(suspended);
        });
    }

    void await_resume() {}

  private:
    template <typename R, typename P>
    explicit sleep_future(const std::chrono::duration<R, P>& timeout)
        : timer_(detail::context(), timeout) {}

    template <typename D>
    explicit sleep_future(const std::chrono::time_point<Clock, D>& timeout)
        : timer_(detail::context(), timeout) {}

    asio::basic_waitable_timer<Clock> timer_;
    bool done_{false};

    template <typename R, typename P>
    friend sleep_future<std::chrono::high_resolution_clock> sleep_for(
        const std::chrono::duration<R, P>&);

    template <typename C, typename D>
    friend sleep_future<C> sleep_until(const std::chrono::time_point<C, D>&);
};

/// @ingroup sleep_grp
template <typename Rep, typename Period>
sleep_future<std::chrono::high_resolution_clock> sleep_for(
    const std::chrono::duration<Rep, Period>& timeout) {
    return sleep_future<std::chrono::high_resolution_clock>{timeout};
}

/// @ingroup sleep_grp
template <typename Clock, typename Dur>
sleep_future<Clock> sleep_until(
    const std::chrono::time_point<Clock, Dur>& timeout) {
    return sleep_future<Clock>{timeout};
}

} // namespace crasy

#endif
