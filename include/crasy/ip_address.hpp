#ifndef CRASY_IP_ADDRESS_HPP
#define CRASY_IP_ADDRESS_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
#include <asio/ip/address.hpp>
#include <asio/ip/address_v4.hpp>
#include <asio/ip/address_v6.hpp>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include <iostream>
#include <span>
#include <string_view>

namespace crasy {

class CRASY_API ipv4_address {
  public:
    ipv4_address() = default;
    explicit ipv4_address(const asio::ip::address_v4& addr);
    explicit ipv4_address(std::string_view addr);
    explicit ipv4_address(std::span<const unsigned char> bytes);
    explicit ipv4_address(std::span<const unsigned char, 4> bytes);
    explicit ipv4_address(std::span<const std::byte> bytes);
    explicit ipv4_address(std::span<const std::byte, 4> bytes);
    explicit ipv4_address(uint_least32_t uint_val);

    ipv4_address(const ipv4_address&) = default;
    ipv4_address(ipv4_address&&) = default;
    ~ipv4_address() = default;
    ipv4_address& operator=(const ipv4_address&) = default;
    ipv4_address& operator=(ipv4_address&&) = default;

    static ipv4_address any();
    static ipv4_address loopback();
    static ipv4_address broadcast();

    bool is_loopback() const;
    bool is_multicast() const;
    bool is_unspecified() const;

    explicit operator std::array<unsigned char, 4>() const;
    std::array<unsigned char, 4> to_uchar_bytes() const;

    explicit operator std::array<std::byte, 4>() const;
    std::array<std::byte, 4> to_std_bytes() const;

    explicit operator std::string() const;
    std::string to_string() const;

    explicit operator uint_least32_t() const;
    uint_least32_t to_uint() const;

    asio::ip::address_v4& asio_address();
    const asio::ip::address_v4& asio_address() const;

  private:
    asio::ip::address_v4 addr_;
};

class CRASY_API ipv6_address {
  public:
    ipv6_address() = default;
    explicit ipv6_address(const asio::ip::address_v6& addr);
    explicit ipv6_address(std::string_view addr);
    explicit ipv6_address(std::span<const unsigned char> bytes);
    explicit ipv6_address(std::span<const unsigned char, 16> bytes);
    explicit ipv6_address(std::span<const std::byte> bytes);
    explicit ipv6_address(std::span<const std::byte, 16> bytes);
    explicit ipv6_address(const ipv4_address& addr);

    ipv6_address(const ipv6_address&) = default;
    ipv6_address(ipv6_address&&) = default;
    ~ipv6_address() = default;
    ipv6_address& operator=(const ipv6_address&) = default;
    ipv6_address& operator=(ipv6_address&&) = default;

    static ipv6_address any();
    static ipv6_address loopback();

    bool is_link_local() const;
    bool is_loopback() const;
    bool is_multicast() const;
    bool is_multicast_global() const;
    bool is_multicast_link_local() const;
    bool is_multicast_node_local() const;
    bool is_multicast_org_local() const;
    bool is_multicast_site_local() const;
    bool is_site_local() const;
    bool is_unspecified() const;
    bool is_v4_compatible() const;
    bool is_v4_mapped() const;

    uint_least32_t scope_id() const;
    void scope_id(uint_least32_t id);

    asio::ip::address_v6& asio_address();
    const asio::ip::address_v6& asio_address() const;

    explicit operator std::array<unsigned char, 16>() const;
    std::array<unsigned char, 16> to_uchar_bytes() const;

    explicit operator std::array<std::byte, 16>() const;
    std::array<std::byte, 16> to_std_bytes() const;

    explicit operator std::string() const;
    std::string to_string() const;

    explicit operator ipv4_address() const;
    ipv4_address to_v4() const;

  private:
    asio::ip::address_v6 addr_;
};

class CRASY_API ip_address {
  public:
    ip_address() = default;
    explicit ip_address(const asio::ip::address& addr);
    ip_address(const ipv4_address& addr);
    ip_address(const ipv6_address& addr);
    ip_address(std::string_view addr);
    ip_address(std::span<const unsigned char> bytes);
    ip_address(std::span<const unsigned char, 4> bytes);
    ip_address(std::span<const unsigned char, 16> bytes);
    ip_address(std::span<const std::byte> bytes);
    ip_address(std::span<const std::byte, 4> bytes);
    ip_address(std::span<const std::byte, 16> bytes);
    ip_address(uint_least32_t uint_val);

    ip_address(const ip_address&) = default;
    ip_address(ip_address&&) = default;
    ~ip_address() = default;
    ip_address& operator=(const ip_address&) = default;
    ip_address& operator=(ip_address&&) = default;

    bool is_loopback() const;
    bool is_multicast() const;
    bool is_unspecified() const;

    bool is_v4() const;
    bool is_v6() const;

    explicit operator std::string() const;
    std::string to_string() const;

    explicit operator ipv4_address() const;
    ipv4_address to_v4() const;

    explicit operator ipv6_address() const;
    ipv6_address to_v6() const;

    asio::ip::address& asio_address();
    const asio::ip::address& asio_address() const;

  private:
    asio::ip::address addr_;
};

CRASY_API std::strong_ordering operator<=>(const ipv4_address& lhs,
                                           const ipv4_address& rhs);

CRASY_API std::ostream& operator<<(std::ostream& os, const ipv4_address& addr);

CRASY_API std::strong_ordering operator<=>(const ipv6_address& lhs,
                                           const ipv6_address& rhs);

CRASY_API std::ostream& operator<<(std::ostream& os, const ipv6_address& addr);

CRASY_API std::partial_ordering operator<=>(const ip_address& lhs,
                                            const ip_address& rhs);

CRASY_API std::ostream& operator<<(std::ostream& os, const ip_address& addr);

namespace literals {

CRASY_API ip_address operator""_ip(const char* addr);

CRASY_API ipv4_address operator""_ipv4(const char* addr);

CRASY_API ipv6_address operator""_ipv6(const char* addr);

} // namespace literals

} // namespace crasy

#endif
