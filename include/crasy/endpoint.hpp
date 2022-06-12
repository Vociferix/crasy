#ifndef CRASY_ENDPOINT_HPP
#define CRASY_ENDPOINT_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <crasy/ip_address.hpp>
#include <cstdint>

namespace crasy {

using port_type = uint_least16_t;

class endpoint {
  public:
    endpoint() = default;
    endpoint(const endpoint&) = default;
    endpoint(endpoint&&) = default;
    ~endpoint() = default;
    endpoint& operator=(const endpoint&) = default;
    endpoint& operator=(endpoint&&) = default;

    endpoint(const ip_address& addr, port_type port)
        : addr_(addr), port_(port) {}

    ip_address& address() { return addr_; }
    const ip_address& address() const { return addr_; }

    port_type& port() { return port_; }
    port_type port() const { return port_; }

  private:
    ip_address addr_;
    port_type port_{0};
};

} // namespace crasy

#endif
