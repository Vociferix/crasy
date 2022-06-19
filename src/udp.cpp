#include <crasy/io_future.hpp>
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

struct connect_future : public detail::io_future {
    option<result<void>> ret;

    void start(asio::ip::udp::socket& socket,
               const asio::ip::udp::endpoint& ep) {
        socket.async_connect(ep, [this](const auto& ec) {
            if (ec) {
                ret.emplace(err(ec));
            } else {
                ret.emplace(ok());
            }
            this->finish();
        });
    }

    result<void> await_resume() { return *std::move(ret); }
};

future<result<void>> udp_socket::ensure_open(bool is_v4) {
    if (!sock_.is_open()) {
        std::error_code ec;
        co_await spawn_blocking([this, &ec, is_v4] {
            sock_.open(is_v4 ? asio::ip::udp::v4() : asio::ip::udp::v6(), ec);
        });
        if (ec) { co_return err(std::move(ec)); }
    }
    co_return ok();
}

future<result<void>> udp_socket::bind_remote(const endpoint& remote_endpoint) {
    connect_future fut{};
    asio::ip::udp::endpoint ep(remote_endpoint.address().asio_address(),
                               remote_endpoint.port());
    auto ret = co_await ensure_open(remote_endpoint.address().is_v4());
    if (ret.is_err()) { co_return ret; }
    fut.start(sock_, ep);
    co_return co_await fut;
}

option<const endpoint&> udp_socket::local_endpoint() const {
    return local_.map([](const auto& ep) -> const endpoint& { return ep; });
}

option<const endpoint&> udp_socket::remote_endpoint() const {
    return remote_.map([](const auto& ep) -> const endpoint& { return ep; });
}

struct send_future : public detail::io_future {
    option<result<std::size_t>> ret;

    template <typename T>
    void start(asio::ip::udp::socket& socket, std::span<const T> buffer) {
        socket.async_send(asio_buffer(buffer),
                          [this](const auto& ec, auto cnt) {
                              if (ec) {
                                  ret.emplace(err(ec));
                              } else {
                                  ret.emplace(ok(cnt));
                              }
                              this->finish();
                          });
    }

    result<std::size_t> await_resume() { return *std::move(ret); }
};

future<result<std::size_t>> udp_socket::send(
    std::span<const std::byte> buffer) {
    send_future fut{};
    fut.start(sock_, buffer);
    co_return co_await fut;
}

struct send_to_future : public detail::io_future {
    option<result<std::size_t>> ret;

    template <typename T>
    void start(asio::ip::udp::socket& socket,
               std::span<const T> buffer,
               const asio::ip::udp::endpoint& peer) {
        socket.async_send_to(asio_buffer(buffer), peer,
                             [this](const auto& ec, auto cnt) {
                                 if (ec) {
                                     ret.emplace(err(ec));
                                 } else {
                                     ret.emplace(ok(cnt));
                                 }
                                 this->finish();
                             });
    }

    result<std::size_t> await_resume() { return *std::move(ret); }
};

future<result<std::size_t>> udp_socket::send_to(
    std::span<const std::byte> buffer,
    const endpoint& peer) {
    send_to_future fut{};
    asio::ip::udp::endpoint ep(peer.address().asio_address(), peer.port());
    auto ret = co_await ensure_open(peer.address().is_v4());
    if (ret.is_err()) { co_return std::move(ret).propagate(); }
    fut.start(sock_, buffer, ep);
    co_return co_await fut;
}

struct recv_future : public detail::io_future {
    option<result<std::size_t>> ret;

    template <typename T>
    void start(asio::ip::udp::socket& socket, std::span<T> buffer) {
        socket.async_receive(asio_buffer(buffer),
                             [this](const auto& ec, auto cnt) {
                                 if (ec) {
                                     ret.emplace(err(ec));
                                 } else {
                                     ret.emplace(ok(cnt));
                                 }
                                 this->finish();
                             });
    }

    result<std::size_t> await_resume() { return *std::move(ret); }
};

future<result<std::size_t>> udp_socket::recv(std::span<std::byte> buffer) {
    recv_future fut{};
    fut.start(sock_, buffer);
    co_return co_await fut;
}

struct recv_from_future : public detail::io_future {
    option<result<std::size_t>> ret;

    template <typename T>
    void start(asio::ip::udp::socket& socket,
               std::span<T> buffer,
               asio::ip::udp::endpoint& asio_ep) {
        socket.async_receive_from(asio_buffer(buffer), asio_ep,
                                  [this](const auto& ec, auto cnt) {
                                      if (ec) {
                                          ret.emplace(err(ec));
                                      } else {
                                          ret.emplace(ok(cnt));
                                      }
                                      this->finish();
                                  });
    }

    result<std::size_t> await_resume() { return *std::move(ret); }
};

future<result<std::size_t>> recv_from_impl(asio::ip::udp::socket& sock,
                                           std::span<std::byte> buffer,
                                           asio::ip::udp::endpoint& peer) {
    recv_from_future fut{};
    fut.start(sock, buffer, peer);
    co_return co_await fut;
}

future<result<std::size_t>> udp_socket::recv_from(std::span<std::byte> buffer,
                                                  endpoint& peer) {
    auto ret = co_await ensure_open(peer.address().is_v4());
    if (ret.is_err()) { co_return std::move(ret).propagate(); }

    asio::ip::udp::endpoint ep{peer.address().asio_address(), peer.port()};
    auto res = co_await recv_from_impl(sock_, buffer, ep);
    peer.address() = ip_address(ep.address());
    peer.port() = ep.port();
    co_return res;
}

struct wait_future : public detail::io_future {
    option<result<void>> ret;

    void start(asio::ip::udp::socket& sock,
               asio::ip::udp::socket::wait_type type) {
        sock.async_wait(type, [this](const auto& ec) {
            if (ec) {
                ret.emplace(err(ec));
            } else {
                ret.emplace(ok());
            }
            this->finish();
        });
    }

    result<void> await_resume() { return *std::move(ret); }
};

future<result<void>> udp_socket::wait_read() {
    wait_future fut{};
    fut.start(sock_, asio::ip::udp::socket::wait_read);
    co_return co_await fut;
}

future<result<void>> udp_socket::wait_write() {
    wait_future fut{};
    fut.start(sock_, asio::ip::udp::socket::wait_write);
    co_return co_await fut;
}

result<std::size_t> udp_socket::available() const {
    std::error_code ec;
    auto ret = sock_.available(ec);
    if (ec) {
        return err(std::move(ec));
    } else {
        return ok(ret);
    }
}

} // namespace crasy
