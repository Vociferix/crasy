#include <crasy/spawn_blocking.hpp>
#include <crasy/udp.hpp>
#include "internal.hpp"

namespace crasy {

udp_socket::udp_socket() : sock_(detail::context()) {}

future<result<void>> udp_socket::bind_local(const endpoint& local_endpoint) {
    co_return co_await spawn_blocking([this,
                                       &local_endpoint]() -> result<void> {
        asio::ip::udp::endpoint ep(local_endpoint.address().asio_address(),
                                   local_endpoint.port());
        std::error_code ec;
        if (!sock_.is_open()) {
            sock_.open(local_endpoint.address().is_v4() ? asio::ip::udp::v4() :
                                                          asio::ip::udp::v6(),
                       ec);
            if (ec) { return err(std::move(ec)); }
        }
        sock_.bind(ep, ec);
        if (ec) {
            return err(std::move(ec));
        } else {
            local_.emplace(ip_address(ep.address()), ep.port());
            return ok();
        }
    });
}

struct connect_future {
    option<result<void>> ret_{};
    std::coroutine_handle<> suspended_{};

    void start(asio::ip::udp::socket& socket,
               const asio::ip::udp::endpoint& ep) {
        socket.async_connect(ep, [this](const auto& ec) {
            if (ec) {
                ret_.emplace(err(ec));
            } else {
                ret_.emplace(ok());
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

static future<result<void>> ensure_open(asio::ip::udp::socket& sock,
                                        bool is_v4) {
    if (!sock.is_open()) {
        std::error_code ec;
        co_await spawn_blocking([&sock, &ec, is_v4] {
            sock.open(is_v4 ? asio::ip::udp::v4() : asio::ip::udp::v6(), ec);
        });
        if (ec) { co_return err(std::move(ec)); }
    }
    co_return ok();
}

future<result<void>> udp_socket::bind_remote(const endpoint& remote_endpoint) {
    connect_future fut{};
    asio::ip::udp::endpoint ep(remote_endpoint.address().asio_address(),
                               remote_endpoint.port());
    auto ret = co_await ensure_open(sock_, remote_endpoint.address().is_v4());
    if (ret.is_err()) { co_return ret; }
    fut.start(sock_, ep);
    co_await fut;
    co_return *std::move(fut.ret_);
}

option<const endpoint&> udp_socket::local_endpoint() const {
    return local_.map([](const auto& ep) -> const endpoint& { return ep; });
}

option<const endpoint&> udp_socket::remote_endpoint() const {
    return remote_.map([](const auto& ep) -> const endpoint& { return ep; });
}

struct send_future {
    option<result<std::size_t>> ret_{};
    std::coroutine_handle<> suspended_{};

    template <typename T>
    void start(asio::ip::udp::socket& socket, std::span<const T> buffer) {
        socket.async_send(asio_buffer(buffer),
                          [this](const auto& ec, auto cnt) {
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

future<result<std::size_t>> udp_socket::send(
    std::span<const unsigned char> buffer) {
    send_future fut{};
    fut.start(sock_, buffer);
    co_await fut;
    co_return *std::move(fut.ret_);
}

future<result<std::size_t>> udp_socket::send(
    std::span<const std::byte> buffer) {
    send_future fut{};
    fut.start(sock_, buffer);
    co_await fut;
    co_return *std::move(fut.ret_);
}

struct send_to_future {
    option<result<std::size_t>> ret_{};
    std::coroutine_handle<> suspended_{};

    template <typename T>
    void start(asio::ip::udp::socket& socket,
               std::span<const T> buffer,
               const asio::ip::udp::endpoint& peer) {
        socket.async_send_to(asio_buffer(buffer), peer,
                             [this](const auto& ec, auto cnt) {
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

future<result<std::size_t>> udp_socket::send_to(
    std::span<const unsigned char> buffer,
    const endpoint& peer) {
    send_to_future fut{};
    asio::ip::udp::endpoint ep(peer.address().asio_address(), peer.port());
    auto ret = co_await ensure_open(sock_, peer.address().is_v4());
    if (ret.is_err()) { co_return std::move(ret).propagate(); }
    fut.start(sock_, buffer, ep);
    co_await fut;
    co_return *std::move(fut.ret_);
}

future<result<std::size_t>> udp_socket::send_to(
    std::span<const std::byte> buffer,
    const endpoint& peer) {
    send_to_future fut{};
    asio::ip::udp::endpoint ep(peer.address().asio_address(), peer.port());
    auto ret = co_await ensure_open(sock_, peer.address().is_v4());
    if (ret.is_err()) { co_return std::move(ret).propagate(); }
    fut.start(sock_, buffer, ep);
    co_await fut;
    co_return *std::move(fut.ret_);
}

struct recv_future {
    option<result<std::size_t>> ret_{};
    std::coroutine_handle<> suspended_{};

    template <typename T>
    void start(asio::ip::udp::socket& socket, std::span<T> buffer) {
        socket.async_receive(asio_buffer(buffer),
                             [this](const auto& ec, auto cnt) {
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

future<result<std::size_t>> udp_socket::recv(std::span<unsigned char> buffer) {
    recv_future fut{};
    fut.start(sock_, buffer);
    co_await fut;
    co_return *std::move(fut.ret_);
}

future<result<std::size_t>> udp_socket::recv(std::span<std::byte> buffer) {
    recv_future fut{};
    fut.start(sock_, buffer);
    co_await fut;
    co_return *std::move(fut.ret_);
}

struct recv_from_future {
    asio::ip::udp::endpoint& asio_ep_;
    option<result<std::size_t>> ret_{};
    std::coroutine_handle<> suspended_{};

    template <typename T>
    void start(asio::ip::udp::socket& socket, std::span<T> buffer) {
        socket.async_receive_from(
            asio_buffer(buffer), asio_ep_, [this](const auto& ec, auto cnt) {
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

future<result<std::size_t>> udp_socket::recv_from(
    std::span<unsigned char> buffer,
    endpoint& peer) {
    asio::ip::udp::endpoint ep{peer.address().asio_address(), peer.port()};
    recv_from_future fut{ep};
    auto ret = co_await ensure_open(sock_, peer.address().is_v4());
    if (ret.is_err()) { co_return std::move(ret).propagate(); }
    fut.start(sock_, buffer);
    co_await fut;
    peer.address() = ip_address(ep.address());
    peer.port() = ep.port();
    co_return *std::move(fut.ret_);
}

future<result<std::size_t>> udp_socket::recv_from(std::span<std::byte> buffer,
                                                  endpoint& peer) {
    asio::ip::udp::endpoint ep{peer.address().asio_address(), peer.port()};
    recv_from_future fut{ep};
    auto ret = co_await ensure_open(sock_, peer.address().is_v4());
    if (ret.is_err()) { co_return std::move(ret).propagate(); }
    fut.start(sock_, buffer);
    co_await fut;
    peer.address() = ip_address(ep.address());
    peer.port() = ep.port();
    co_return *std::move(fut.ret_);
}

} // namespace crasy
