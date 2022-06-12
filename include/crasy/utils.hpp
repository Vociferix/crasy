#ifndef CRASY_UTILS_HPP
#define CRASY_UTILS_HPP

// clang-format off
#include <crasy/config.hpp>
// clang-format on

#include <crasy/result.hpp>

namespace crasy {

CRASY_API result<std::size_t> available_cpu_cores();

}

#endif
