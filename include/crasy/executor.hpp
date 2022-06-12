#ifndef CRASY_EXECUTOR_HPP
#define CRASY_EXECUTOR_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <crasy/future.hpp>
#include <crasy/lfqueue.hpp>

#include <asio/io_context.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace crasy {

class CRASY_API executor {
  public:
    executor();
    explicit executor(std::size_t core_threads);
    executor(std::size_t core_threads, std::size_t max_threads);

    executor(const executor&) = delete;
    executor(executor&&) = delete;

    ~executor();

    executor& operator=(const executor&) = delete;
    executor& operator=(executor&&) = delete;

    template <typename F>
    decltype(auto) block_on(F&& func) {
        using fut_t = decltype(func());
        if constexpr (std::is_same_v<fut_t, future<void>>) {
            if constexpr (std::is_pointer_v<std::remove_reference_t<F>>) {
                block_on_impl(&executor::async_call<std::remove_reference_t<F>>,
                              reinterpret_cast<void*>(func));
            } else if constexpr (std::is_function_v<
                                     std::remove_reference_t<F>>) {
                block_on_impl(
                    &executor::async_call<std::remove_reference_t<F>*>,
                    reinterpret_cast<void*>(&func));
            } else {
                block_on_impl(&executor::async_call<std::remove_reference_t<F>>,
                              &func);
            }
        } else {
            option<typename fut_t::return_type> ret;
            block_on([f = std::forward<F>(func), &ret]() -> future<void> {
                ret.emplace(co_await f());
            });
            return *std::move(ret);
        }
    }

  private:
    void schedule_task(std::coroutine_handle<> task);
    void run_blocking(void (*func)(void*), void* data);
    void core_work();
    void blocking_work();

    void block_on_impl(future<void> (*func)(void*), void* data);

    template <typename F>
    static future<void> async_call(void* func) {
        if constexpr (std::is_pointer_v<F>) {
            return reinterpret_cast<F>(func)();
        } else {
            return (*reinterpret_cast<F*>(func))();
        }
    }

    struct blocking_task {
        void (*func)(void*);
        void* data;
    };

    asio::io_context context_;
    std::vector<std::thread> core_workers_;
    std::vector<std::thread> blocking_workers_;
    std::size_t max_blocking_workers_{0};
    lfqueue<blocking_task> blocking_tasks_;
    std::mutex core_mut_;
    std::condition_variable core_cv_;
    std::mutex blocking_mut_;
    std::condition_variable blocking_cv_;
    std::atomic<size_t> blocking_waiting_{0};
    bool core_work_pending_{false};
    bool core_done_{false};
    bool blocking_done_{false};

    friend void detail::schedule_task(std::coroutine_handle<>);
    friend void detail::run_blocking(void (*func)(void*), void* data);
    friend asio::io_context& detail::context();
};

} // namespace crasy

#endif
