#include <crasy/executor.hpp>
#include <crasy/utils.hpp>

#include <cassert>
#include <limits>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <asio.hpp>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

namespace crasy {

static thread_local executor* g_exec = nullptr;

class exec_guard {
  public:
    explicit exec_guard(executor& exec) {
        if (g_exec == nullptr) {
            root_ = true;
            g_exec = &exec;
        } else if (g_exec != &exec) {
            throw std::runtime_error("attempt to nest executors");
        }
    }

    ~exec_guard() {
        if (root_) { g_exec = nullptr; }
    }

    exec_guard(const exec_guard&) = delete;
    exec_guard(exec_guard&&) = delete;
    exec_guard& operator=(const exec_guard&) = delete;
    exec_guard& operator=(exec_guard&&) = delete;

  private:
    bool root_;
};

executor::executor() : executor(*available_cpu_cores()) {}

executor::executor(std::size_t core_threads)
    : executor(core_threads, core_threads) {}

executor::executor(std::size_t core_threads, std::size_t blocking_threads) {
    if (core_threads == 0) {
        throw std::invalid_argument(
            "executor must support at least one core thread");
    }
    if (blocking_threads == 0) {
        throw std::invalid_argument(
            "executor must support at least one blocking thread");
    }
    core_workers_.reserve(core_threads);
    for (std::size_t i = 0; i < core_threads; ++i) {
        core_workers_.emplace_back([this] { core_work(); });
    }
    blocking_workers_.reserve(blocking_threads);
    for (std::size_t i = 0; i < blocking_threads; ++i) {
        blocking_workers_.emplace_back([this] { blocking_work(); });
    }
}

executor::~executor() {
    {
        std::lock_guard<std::mutex> lock{blocking_mut_};
        blocking_done_ = true;
    }
    blocking_cv_.notify_all();
    for (auto& worker : blocking_workers_) { worker.join(); }

    {
        std::lock_guard<std::mutex> lock{core_mut_};
        core_done_ = true;
    }
    core_cv_.notify_all();
    for (auto& worker : core_workers_) { worker.join(); }
}

void executor::block_on_impl(future<void> (*func)(void*), void* data) {
    std::mutex mut;
    std::condition_variable cv;
    bool done = false;
    std::coroutine_handle<future<void>::promise_type> handle;
    auto work_guard = asio::make_work_guard(context_);
    asio::post(context_, [this, func, data, &mut, &cv, &done, &handle] {
        handle = [](auto fn, auto ptr, auto& mtx, auto& cond_var,
                    auto& dn) -> future<void> {
            co_await fn(ptr);
            {
                std::lock_guard<std::mutex> lock{mtx};
                dn = true;
            }
            cond_var.notify_one();
        }(func, data, mut, cv, done)
                                     .into_handle();
    });
    {
        std::lock_guard<std::mutex> lock{core_mut_};
        core_work_pending_ = true;
    }
    core_cv_.notify_one();
    {
        std::unique_lock<std::mutex> lock{mut};
        cv.wait(lock, [&done] { return done; });
    }
    [[maybe_unused]] future<void> guard{handle};
}

void executor::schedule_task(std::coroutine_handle<> task) {
    assert(task && !task.done());
#ifndef NDEBUG
    asio::post(context_, [=] {
        assert(task && !task.done());
        task.resume();
    });
#else
    asio::post(context_, task);
#endif
}

void executor::run_blocking(void (*func)(void*), void* data) {
    blocking_tasks_.push(blocking_task{func, data});
    blocking_waiting_.fetch_add(1, std::memory_order_relaxed);
    if (blocking_mut_.try_lock()) { blocking_mut_.unlock(); }
    blocking_cv_.notify_one();
}

void executor::core_work() {
    exec_guard ex{*this};
    for (;;) {
        {
            std::unique_lock<std::mutex> lock{core_mut_};
            core_cv_.wait(lock,
                          [this] { return core_work_pending_ || core_done_; });
            if (core_done_) { break; }
            core_work_pending_ = false;
        }
        context_.run();
    }
}

void executor::blocking_work() {
    exec_guard ex{*this};
    for (;;) {
        auto task = blocking_tasks_.pop();
        if (task.has_value()) {
            blocking_waiting_.fetch_sub(1, std::memory_order_relaxed);
            task->func(task->data);
        } else {
            std::unique_lock<std::mutex> lock{blocking_mut_};
            blocking_cv_.wait(lock, [this] {
                return blocking_done_ ||
                       blocking_waiting_.load(std::memory_order_relaxed) > 0;
            });
            if (blocking_done_) { break; }
        }
    }
}

namespace detail {

bool in_executor_context() { return g_exec != nullptr; }

void schedule_task(std::coroutine_handle<> handle) {
    if (g_exec == nullptr) {
        throw std::runtime_error(
            "attempt to execute async task outside of executor context");
    }
    g_exec->schedule_task(std::move(handle));
}

asio::io_context& context() {
    if (g_exec == nullptr) {
        throw std::runtime_error(
            "attempt to access async I/O context outside of executor context");
    }
    return g_exec->context_;
}

void run_blocking(void (*func)(void*), void* user_data) {
    if (g_exec == nullptr) {
        throw std::runtime_error(
            "attempt to spawn blocking task outside of executor context");
    }
    g_exec->run_blocking(func, user_data);
}

} // namespace detail

} // namespace crasy
