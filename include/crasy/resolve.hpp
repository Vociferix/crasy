#ifndef CRASY_RESOLVE_HPP
#define CRASY_RESOLVE_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <crasy/endpoint.hpp>
#include <crasy/future.hpp>
#include <crasy/result.hpp>

#include <string_view>

namespace crasy {

/// @ingroup resolve_grp
CRASY_API future<result<ip_address>> resolve_one(std::string_view host);

/// @ingroup resolve_grp
CRASY_API future<result<ipv4_address>> resolve_one_v4(std::string_view host);

/// @ingroup resolve_grp
CRASY_API future<result<ipv6_address>> resolve_one_v6(std::string_view host);

/// @ingroup resolve_grp
CRASY_API future<result<std::vector<ip_address>>> resolve(
    std::string_view host);

/// @ingroup resolve_grp
CRASY_API future<result<std::vector<ipv4_address>>> resolve_v4(
    std::string_view host);

/// @ingroup resolve_grp
CRASY_API future<result<std::vector<ipv6_address>>> resolve_v6(
    std::string_view host);

} // namespace crasy

#endif
