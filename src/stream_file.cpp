#include <crasy/spawn_blocking.hpp>
#include <crasy/stream_file.hpp>

namespace crasy {

stream_file::stream_file() : file_(detail::context()) {}

stream_file::~stream_file() {
    if (is_open()) {
        spawn_blocking([this] { file_.close(); }).detach();
    }
}

future<result<void>> stream_file::open(const std::filesystem::path& path,
                                       flags open_flags) {
    auto guard = co_await lock(read_mtx_, write_mtx_);
    co_return co_await spawn_blocking([this, &path, flags] {
        std::error_code ec;
        file_.open(path.string(), flags, ec);
        if (ec) {
            return err(ec);
        } else {
            return ok();
        }
    });
}

future<result<void>> stream_file::close() {
    auto guard = co_await lock(read_mtx_, write_mtx_);
    co_return co_await spawn_blocking([this] {
        std::error_code ec;
        file_.close(ec);
        if (ec) {
            return err(ec);
        } else {
            return ok();
        }
    });
}

bool stream_file::is_open() const { return file_.is_open(); }

stream_file::operator bool() const { return file_.is_open(); }

template <typename T>
struct read_some_future {
    asio::stream_file& file_;
    std::span<T> buffer_;
    option<result<std::size_t>> ret_{};
    std::coroutine_handle<> suspended_{};

    void start() {
        file_.async_read_some(buffer_, [this](const auto& ec, auto cnt) {
            if (ec) {
                ret_.emplace(err(ec));
            } else {
                ret_.emplace(ok(cnt));
            }
            if (suspended_) {
                auto suspended = suspended_;
                suspended_ = std::coroutine_handle<>();
                detail::schedule_task(suspended);
            }
        });
    }

    bool await_ready() { return ret_.has_value(); }

    void await_suspend(std::coroutine_handle<> suspended) {
        if (ret_.has_value()) {
            detail::schedule_task(suspended);
        } else {
            suspended_ = suspended;
        }
    }

    void await_resume() {}
};

template <typename T>
future<result<std::size_t>> read_some_impl(asio::stream_file& file,
                                           mutex& mtx,
                                           std::span<T> buffer) {
    read_some_future<T> fut{file, buffer};
    {
        auto guard = co_await lock(mtx);
        fut.start();
        co_await fut;
    }
    co_return *std::move(fut.ret);
}

future<result<std::size_t>> stream_file::read_some(std::span<char> buffer) {
    return read_some_impl(file_, read_mtx_, buffer);
}

future<result<std::size_t>> stream_file::read_some(
    std::span<unsigned char> buffer) {
    return read_some_impl(file_, read_mtx_, buffer);
}

future<result<std::size_t>> stream_file::read_some(
    std::span<std::byte> buffer) {
    return read_some_impl(file_, read_mtx_, buffer);
}

template <typename T>
struct read_future {
    asio::stream_file& file_;
    std::span<T> buffer_;
    std::size_t count_{0};
    option<result<std::size_t>> ret_{};
    std::coroutine_handle<> suspended_{};

    void finish() {
        if (suspended_) {
            auto suspended = suspended_;
            suspended_ = std::coroutine_handle<>();
            detail::schedule_task(suspended);
        }
    }

    void proceed(const std::error_code& ec, std::size_t count) {
        if (ec) {
            ret_.emplace(err(ec));
            finish();
        } else if (count == 0) {
            ret_.emplace(ok(count_));
            finish();
        } else if (count == buffer_.size()) {
            ret_.emplace(ok(count_ + count));
            finish();
        } else {
            buffer_ = buffer_.subspan(count);
            file_.async_read_some(buffer_, [this](const auto& ec, auto cnt) {
                proceed(ec, cnt);
            });
        }
    }

    void start() {
        if (buffer_.empty()) {
            ret_.emplace(ok(0));
        } else {
            file_.async_read_some(buffer_, [this](const auto& ec, auto cnt) {
                proceed(ec, cnt);
            });
        }
    }

    bool await_ready() { return ret_.has_value(); }

    void await_suspend(std::coroutine_handle<> suspended) {
        if (ret_.has_value()) {
            detail::schedule_task(suspended);
        } else {
            suspended_ = suspended;
        }
    }

    void await_resume() {}
};

template <typename T>
future<result<std::size_t>> read_impl(asio::stream_file& file,
                                      mutex& mtx,
                                      std::span<T> buffer) {
    read_future<T> fut{file, buffer};
    {
        auto guard = co_await lock(mtx);
        fut.start();
        co_await fut;
    }
    co_return *std::move(fut.ret);
}

future<result<std::size_t>> stream_file::read(std::span<char> buffer) {
    return read_impl(file_, read_mtx_, buffer);
}

future<result<std::size_t>> stream_file::read(std::span<unsigned char> buffer) {
    return read_impl(file_, read_mtx_, buffer);
}

future<result<std::size_t>> stream_file::read(std::span<std::byte> buffer) {
    return read_impl(file_, read_mtx_, buffer);
}

template <typename T>
struct write_some_future {
    asio::stream_file& file_;
    std::span<const T> buffer_;
    option<result<std::size_t>> ret_{};
    std::coroutine_handle<> suspended_{};

    void start() {
        file_.async_write_some(buffer_, [this](const auto& ec, auto cnt) {
            if (ec) {
                ret_.emplace(err(ec));
            } else {
                ret_.emplace(ok(cnt));
            }
            if (suspended_) {
                auto suspended = suspended_;
                suspended_ = std::coroutine_handle<>();
                detail::schedule_task(suspended);
            }
        });
    }

    bool await_ready() { return ret_.has_value(); }

    void await_suspend(std::coroutine_handle<> suspended) {
        if (ret_.has_value()) {
            detail::schedule_task(suspended);
        } else {
            suspended_ = suspended;
        }
    }

    void await_resume() {}
};

template <typename T>
future<result<std::size_t>> write_some_impl(asio::stream_file& file,
                                            mutex& mtx,
                                            std::span<const T> buffer) {
    write_some_future<T> fut{file, buffer};
    {
        auto guard = co_await lock(mtx);
        fut.start();
        co_await fut;
    }
    co_return *std::move(fut.ret);
}

future<result<std::size_t>> stream_file::write_some(
    std::span<const char> buffer) {
    return write_some_impl(file_, write_mtx_, buffer);
}

future<result<std::size_t>> stream_file::write_some(
    std::span<const unsigned char> buffer) {
    return write_some_impl(file_, write_mtx_, buffer);
}

future<result<std::size_t>> stream_file::write_some(
    std::span<const std::byte> buffer) {
    return write_some_impl(file_, write_mtx_, buffer);
}

template <typename T>
struct write_future {
    asio::stream_file& file_;
    std::span<const T> buffer_;
    std::size_t count_{0};
    option<result<std::size_t>> ret_{};
    std::coroutine_handle<> suspended_{};

    void finish() {
        if (suspended_) {
            auto suspended = suspended_;
            suspended_ = std::coroutine_handle<>();
            detail::schedule_task(suspended);
        }
    }

    void proceed(const std::error_code& ec, std::size_t count) {
        if (ec) {
            ret_.emplace(err(ec));
            finish();
        } else if (count == 0) {
            ret_.emplace(ok(count_));
            finish();
        } else if (count == buffer_.size()) {
            ret_.emplace(ok(count_ + count));
            finish();
        } else {
            buffer_ = buffer_.subspan(count);
            file_.async_write_some(buffer_, [this](const auto& ec, auto cnt) {
                proceed(ec, cnt);
            });
        }
    }

    void start() {
        if (buffer_.empty()) {
            ret_.emplace(ok(0));
        } else {
            file_.async_write_some(buffer_, [this](const auto& ec, auto cnt) {
                proceed(ec, cnt);
            });
        }
    }

    bool await_writey() { return ret_.has_value(); }

    void await_suspend(std::coroutine_handle<> suspended) {
        if (ret_.has_value()) {
            detail::schedule_task(suspended);
        } else {
            suspended_ = suspended;
        }
    }

    void await_resume() {}
};

template <typename T>
future<result<std::size_t>> write_impl(asio::stream_file& file,
                                       mutex& mtx,
                                       std::span<const T> buffer) {
    write_future<T> fut{file, buffer};
    {
        auto guard = co_await lock(mtx);
        fut.start();
        co_await fut;
    }
    co_return *std::move(fut.ret);
}

future<result<std::size_t>> stream_file::write(std::span<const char> buffer) {
    return write_impl(file_, write_mtx_, buffer);
}

future<result<std::size_t>> stream_file::write(
    std::span<const unsigned char> buffer) {
    return write_impl(file_, write_mtx_, buffer);
}

future<result<std::size_t>> stream_file::write(
    std::span<const std::byte> buffer) {
    return write_impl(file_, write_mtx_, buffer);
}

future<result<void>> stream_file::resize(uint64_t new_size) {
    auto guard = co_await lock(read_mtx_, write_mtx_);
    co_return co_await spawn_blocking([this, new_size]() -> result<void> {
        std::error_code ec;
        file_.resize(new_size, ec);
        if (ec) {
            return err(ec);
        } else {
            return ok();
        }
    });
}

future<result<uint64_t>> stream_file::size() const {
    auto guard = co_await lock(read_mtx_, write_mtx_);
    co_return co_await spawn_blocking([this]() -> result<uint64_t> {
        std::error_code ec;
        auto ret = file_.size(ec);
        if (ec) {
            return err(ec);
        } else {
            return ok(ret);
        }
    });
}

future<result<uint64_t>> stream_file::seek(int64_t offset, seek_basis whence) {
    auto guard = co_await lock(read_mtx_, write_mtx_);
    co_return co_await spawn_blocking(
        [this, offset, whence]() -> result<uint64_t> {
            std::error_code ec;
            auto ret = file_.seek(offset, whence, ec);
            if (ec) {
                return err(ec);
            } else {
                return ok(ret);
            }
        });
}

future<result<void>> stream_file::sync_all() {
    auto guard = co_await lock(read_mtx_, write_mtx_);
    co_return co_await spawn_blocking([this]() -> result<void> {
        std::error_code ec;
        file_.sync_all(ec);
        if (ec) {
            return err(ec);
        } else {
            return ok();
        }
    });
}

future<result<void>> stream_file::sync_data() {
    auto guard = co_await lock(read_mtx_, write_mtx_);
    co_return co_await spawn_blocking([this]() -> result<void> {
        std::error_code ec;
        file_.sync_data(ec);
        if (ec) {
            return err(ec);
        } else {
            return ok();
        }
    });
}

} // namespace crasy
