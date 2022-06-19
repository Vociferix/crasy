#ifndef CRASY_UDP_HPP
#define CRASY_UDP_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <asio/ip/udp.hpp>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include <crasy/endpoint.hpp>
#include <crasy/future.hpp>
#include <crasy/result.hpp>
#include <crasy/utils.hpp>

#include <span>

namespace crasy {

class CRASY_API udp_socket {
  public:
    udp_socket();
    udp_socket(const udp_socket&) = delete;
    udp_socket(udp_socket&&) = default;
    ~udp_socket() = default;
    udp_socket& operator=(const udp_socket&) = delete;
    udp_socket& operator=(udp_socket&&) = default;

    future<result<void>> bind_local(const endpoint& local_endpoint);
    future<result<void>> bind_remote(const endpoint& remote_endpoint);

    option<const endpoint&> local_endpoint() const;
    option<const endpoint&> remote_endpoint() const;

    future<result<void>> wait_read();
    future<result<void>> wait_write();

    result<std::size_t> available() const;

    future<result<std::size_t>> send(std::span<const std::byte> buf);

    future<result<std::size_t>> send(buffer auto const& buf) {
        return send(std::span<const std::byte>(std::as_bytes(std::span(buf))));
    }

    future<result<std::size_t>> send_to(std::span<const std::byte> buf,
                                        const endpoint& peer);

    future<result<std::size_t>> send_to(buffer auto const& buf,
                                        const endpoint& peer) {
        return send_to(
            std::span<const std::byte>(std::as_bytes(std::span(buf))), peer);
    }

    future<result<std::size_t>> recv(std::span<std::byte> buf);

    future<result<std::size_t>> recv(buffer auto& buf) {
        return recv(std::span<std::byte>(std::as_bytes(std::span(buf))));
    }

    future<result<std::size_t>> recv_from(std::span<std::byte> buf,
                                          endpoint& peer);

    future<result<std::size_t>> recv_from(buffer auto& buf, endpoint& peer) {
        return recv_from(std::span<std::byte>(std::as_bytes(std::span(buf))),
                         peer);
    }

  private:
    future<result<void>> ensure_open(bool is_v4);

    asio::ip::udp::socket sock_;
    option<endpoint> local_;
    option<endpoint> remote_;
};

} // namespace crasy

#endif
