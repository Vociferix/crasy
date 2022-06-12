// clang-format off
#include <crasy/config.hpp>
// clang-format on

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

#include <asio/impl/src.hpp>
#ifdef CRASY_HAS_SSL
#include <asio/ssl/impl/src.hpp>
#endif
