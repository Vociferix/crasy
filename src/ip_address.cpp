#include <crasy/ip_address.hpp>

namespace crasy {

ipv4_address::ipv4_address(const asio::ip::address_v4& addr) : addr_(addr) {}

ipv4_address::ipv4_address(std::string_view addr)
    : addr_(asio::ip::make_address_v4(addr)) {}

ipv4_address::ipv4_address(std::span<const unsigned char> bytes) {
    if (bytes.size() != 4) {
        throw std::out_of_range("ipv4_address from bytes");
    }
    *this = ipv4_address(std::span<const unsigned char, 4>(bytes));
}

ipv4_address::ipv4_address(std::span<const unsigned char, 4> bytes)
    : addr_(std::array<unsigned char, 4>{bytes[0], bytes[1], bytes[2],
                                         bytes[3]}) {}

ipv4_address::ipv4_address(std::span<const std::byte> bytes) {
    if (bytes.size() != 4) {
        throw std::out_of_range("ipv4_address from bytes");
    }
    *this = ipv4_address(std::span<const std::byte, 4>(bytes));
}

ipv4_address::ipv4_address(std::span<const std::byte, 4> bytes)
    : addr_(
          std::array<unsigned char, 4>{static_cast<unsigned char>(bytes[0]),
                                       static_cast<unsigned char>(bytes[1]),
                                       static_cast<unsigned char>(bytes[2]),
                                       static_cast<unsigned char>(bytes[3])}) {}

ipv4_address::ipv4_address(uint_least32_t uint_val) : addr_(uint_val) {}

ipv4_address ipv4_address::any() {
    return ipv4_address{asio::ip::address_v4::any()};
}

ipv4_address ipv4_address::loopback() {
    return ipv4_address{asio::ip::address_v4::loopback()};
}

ipv4_address ipv4_address::broadcast() {
    return ipv4_address{asio::ip::address_v4::broadcast()};
}

bool ipv4_address::is_loopback() const { return addr_.is_loopback(); }

bool ipv4_address::is_multicast() const { return addr_.is_multicast(); }

bool ipv4_address::is_unspecified() const { return addr_.is_unspecified(); }

ipv4_address::operator std::array<unsigned char, 4>() const {
    return to_uchar_bytes();
}

std::array<unsigned char, 4> ipv4_address::to_uchar_bytes() const {
    return addr_.to_bytes();
}

ipv4_address::operator std::array<std::byte, 4>() const {
    return to_std_bytes();
}

std::array<std::byte, 4> ipv4_address::to_std_bytes() const {
    auto asio_bytes = addr_.to_bytes();
    auto bytes = std::as_bytes(std::span<const unsigned char, 4>(asio_bytes));
    return {bytes[0], bytes[1], bytes[2], bytes[3]};
}

ipv4_address::operator std::string() const { return to_string(); }

std::string ipv4_address::to_string() const { return addr_.to_string(); }

ipv4_address::operator uint_least32_t() const { return to_uint(); }

uint_least32_t ipv4_address::to_uint() const { return addr_.to_uint(); }

asio::ip::address_v4& ipv4_address::asio_address() { return addr_; }

const asio::ip::address_v4& ipv4_address::asio_address() const { return addr_; }

ipv6_address::ipv6_address(const asio::ip::address_v6& addr) : addr_(addr) {}

ipv6_address::ipv6_address(std::string_view addr)
    : addr_(asio::ip::make_address_v6(addr)) {}

ipv6_address::ipv6_address(std::span<const unsigned char> bytes) {
    if (bytes.size() != 16) {
        throw std::out_of_range("ipv6_address from bytes");
    }
    *this = ipv6_address(std::span<const unsigned char, 16>(bytes));
}

ipv6_address::ipv6_address(std::span<const unsigned char, 16> bytes)
    : addr_(std::array<unsigned char, 16>{
          bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6],
          bytes[7], bytes[8], bytes[9], bytes[10], bytes[11], bytes[12],
          bytes[13], bytes[14], bytes[15]}) {}

ipv6_address::ipv6_address(std::span<const std::byte> bytes) {
    if (bytes.size() != 16) {
        throw std::out_of_range("ipv6_address from bytes");
    }
    *this = ipv6_address(std::span<const std::byte, 16>(bytes));
}

ipv6_address::ipv6_address(std::span<const std::byte, 16> bytes)
    : addr_(std::array<unsigned char, 16>{
          static_cast<unsigned char>(bytes[0]),
          static_cast<unsigned char>(bytes[1]),
          static_cast<unsigned char>(bytes[2]),
          static_cast<unsigned char>(bytes[3]),
          static_cast<unsigned char>(bytes[4]),
          static_cast<unsigned char>(bytes[5]),
          static_cast<unsigned char>(bytes[6]),
          static_cast<unsigned char>(bytes[7]),
          static_cast<unsigned char>(bytes[8]),
          static_cast<unsigned char>(bytes[9]),
          static_cast<unsigned char>(bytes[10]),
          static_cast<unsigned char>(bytes[11]),
          static_cast<unsigned char>(bytes[12]),
          static_cast<unsigned char>(bytes[13]),
          static_cast<unsigned char>(bytes[14]),
          static_cast<unsigned char>(bytes[15])}) {}

ipv6_address::ipv6_address(const ipv4_address& addr)
    : addr_(asio::ip::make_address_v6(asio::ip::v4_mapped,
                                      addr.asio_address())) {}

ipv6_address ipv6_address::any() {
    return ipv6_address{asio::ip::address_v6::any()};
}

ipv6_address ipv6_address::loopback() {
    return ipv6_address{asio::ip::address_v6::loopback()};
}

bool ipv6_address::is_link_local() const { return addr_.is_link_local(); }

bool ipv6_address::is_loopback() const { return addr_.is_loopback(); }

bool ipv6_address::is_multicast() const { return addr_.is_multicast(); }

bool ipv6_address::is_multicast_global() const {
    return addr_.is_multicast_global();
}

bool ipv6_address::is_multicast_link_local() const {
    return addr_.is_multicast_link_local();
}

bool ipv6_address::is_multicast_node_local() const {
    return addr_.is_multicast_node_local();
}

bool ipv6_address::is_multicast_org_local() const {
    return addr_.is_multicast_org_local();
}

bool ipv6_address::is_multicast_site_local() const {
    return addr_.is_multicast_site_local();
}

bool ipv6_address::is_site_local() const { return addr_.is_site_local(); }

bool ipv6_address::is_unspecified() const { return addr_.is_unspecified(); }

bool ipv6_address::is_v4_compatible() const { return addr_.is_v4_compatible(); }

bool ipv6_address::is_v4_mapped() const { return addr_.is_v4_mapped(); }

uint_least32_t ipv6_address::scope_id() const { return addr_.scope_id(); }

void ipv6_address::scope_id(uint_least32_t id) { addr_.scope_id(id); }

asio::ip::address_v6& ipv6_address::asio_address() { return addr_; }

const asio::ip::address_v6& ipv6_address::asio_address() const { return addr_; }

ipv6_address::operator std::array<unsigned char, 16>() const {
    return to_uchar_bytes();
}

std::array<unsigned char, 16> ipv6_address::to_uchar_bytes() const {
    return addr_.to_bytes();
}

ipv6_address::operator std::array<std::byte, 16>() const {
    return to_std_bytes();
}

std::array<std::byte, 16> ipv6_address::to_std_bytes() const {
    auto asio_bytes = addr_.to_bytes();
    auto bytes = std::as_bytes(std::span<unsigned char, 16>(asio_bytes));
    return {bytes[0],  bytes[1],  bytes[2],  bytes[3], bytes[4],  bytes[5],
            bytes[6],  bytes[7],  bytes[8],  bytes[9], bytes[10], bytes[11],
            bytes[12], bytes[13], bytes[14], bytes[15]};
}

ipv6_address::operator std::string() const { return to_string(); }

std::string ipv6_address::to_string() const { return addr_.to_string(); }

ipv6_address::operator ipv4_address() const { return to_v4(); }

ipv4_address ipv6_address::to_v4() const { return ipv4_address{addr_.to_v4()}; }

ip_address::ip_address(const asio::ip::address& addr) : addr_(addr) {}

ip_address::ip_address(const ipv4_address& addr) : addr_(addr.asio_address()) {}

ip_address::ip_address(const ipv6_address& addr) : addr_(addr.asio_address()) {}

ip_address::ip_address(std::string_view addr)
    : addr_(asio::ip::make_address(addr)) {}

ip_address::ip_address(std::span<const unsigned char> bytes) {
    if (bytes.size() == 4) {
        *this = ip_address(std::span<const unsigned char, 4>(bytes));
    } else if (bytes.size() == 16) {
        *this = ip_address(std::span<const unsigned char, 16>(bytes));
    } else {
        throw std::out_of_range("ip_address from bytes");
    }
}

ip_address::ip_address(std::span<const unsigned char, 4> bytes)
    : ip_address(ipv4_address(bytes)) {}

ip_address::ip_address(std::span<const unsigned char, 16> bytes)
    : ip_address(ipv6_address(bytes)) {}

ip_address::ip_address(std::span<const std::byte> bytes) {
    if (bytes.size() == 4) {
        *this = ip_address(std::span<const std::byte, 4>(bytes));
    } else if (bytes.size() == 16) {
        *this = ip_address(std::span<const std::byte, 16>(bytes));
    } else {
        throw std::out_of_range("ip_address from bytes");
    }
}

ip_address::ip_address(std::span<const std::byte, 4> bytes)
    : ip_address(ipv4_address(bytes)) {}

ip_address::ip_address(std::span<const std::byte, 16> bytes)
    : ip_address(ipv6_address(bytes)) {}

ip_address::ip_address(uint_least32_t uint_val)
    : addr_(asio::ip::address_v4(uint_val)) {}

bool ip_address::is_loopback() const { return addr_.is_loopback(); }

bool ip_address::is_multicast() const { return addr_.is_multicast(); }

bool ip_address::is_unspecified() const { return addr_.is_unspecified(); }

bool ip_address::is_v4() const { return addr_.is_v4(); }

bool ip_address::is_v6() const { return addr_.is_v6(); }

ip_address::operator std::string() const { return to_string(); }

std::string ip_address::to_string() const { return addr_.to_string(); }

ip_address::operator ipv4_address() const { return to_v4(); }

ipv4_address ip_address::to_v4() const { return ipv4_address{addr_.to_v4()}; }

ip_address::operator ipv6_address() const { return to_v6(); }

ipv6_address ip_address::to_v6() const { return ipv6_address{addr_.to_v6()}; }

asio::ip::address& ip_address::asio_address() { return addr_; }

const asio::ip::address& ip_address::asio_address() const { return addr_; }

std::strong_ordering operator<=>(const ipv4_address& lhs,
                                 const ipv4_address& rhs) {
    if (lhs.asio_address() < rhs.asio_address()) {
        return std::strong_ordering::less;
    } else if (lhs.asio_address() > rhs.asio_address()) {
        return std::strong_ordering::greater;
    } else {
        return std::strong_ordering::equal;
    }
}

std::ostream& operator<<(std::ostream& os, const ipv4_address& addr) {
    return operator<<(os, addr.asio_address());
}

std::strong_ordering operator<=>(const ipv6_address& lhs,
                                 const ipv6_address& rhs) {
    if (lhs.asio_address() < rhs.asio_address()) {
        return std::strong_ordering::less;
    } else if (lhs.asio_address() > rhs.asio_address()) {
        return std::strong_ordering::greater;
    } else {
        return std::strong_ordering::equal;
    }
}

std::ostream& operator<<(std::ostream& os, const ipv6_address& addr) {
    return operator<<(os, addr.asio_address());
}

std::partial_ordering operator<=>(const ip_address& lhs,
                                  const ip_address& rhs) {
    if (lhs.asio_address() < rhs.asio_address()) {
        return std::partial_ordering::less;
    } else if (lhs.asio_address() > rhs.asio_address()) {
        return std::partial_ordering::greater;
    } else if (lhs.asio_address() == rhs.asio_address()) {
        return std::partial_ordering::equivalent;
    } else {
        return std::partial_ordering::unordered;
    }
}

std::ostream& operator<<(std::ostream& os, const ip_address& addr) {
    return operator<<(os, addr.asio_address());
}

namespace literals {

ip_address operator""_ip(const char* addr) { return ip_address(addr); }

ipv4_address operator""_ipv4(const char* addr) { return ipv4_address(addr); }

ipv6_address operator""_ipv6(const char* addr) { return ipv6_address(addr); }

} // namespace literals

} // namespace crasy
