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

    future<result<std::size_t>> send(std::span<const unsigned char> buffer);
    future<result<std::size_t>> send(std::span<const std::byte> buffer);

    future<result<std::size_t>> send_to(std::span<const unsigned char> buffer,
                                        const endpoint& peer);
    future<result<std::size_t>> send_to(std::span<const std::byte> buffer,
                                        const endpoint& peer);

    future<result<std::size_t>> recv(std::span<unsigned char> buffer);
    future<result<std::size_t>> recv(std::span<std::byte> buffer);

    future<result<std::size_t>> recv_from(std::span<unsigned char> buffer,
                                          endpoint& peer);
    future<result<std::size_t>> recv_from(std::span<std::byte> buffer,
                                          endpoint& peer);

  private:
    asio::ip::udp::socket sock_;
    option<endpoint> local_;
    option<endpoint> remote_;
};

} // namespace crasy

#endif
