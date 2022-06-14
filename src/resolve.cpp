#include <crasy/io_future.hpp>
#include <crasy/resolve.hpp>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <asio/ip/tcp.hpp>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

namespace crasy {

struct resolve_one_any_future : public detail::io_future {
    asio::ip::tcp::resolver resolver;
    option<result<ip_address>> ret;

    resolve_one_any_future() : resolver(detail::context()) {}

    void start(std::string_view host) {
        resolver.async_resolve(
            host, "",
            asio::ip::tcp::resolver::passive |
                asio::ip::tcp::resolver::address_configured,
            [this](const auto& ec, auto results) {
                if (ec) {
                    ret.emplace(err(ec));
                } else if (results.empty()) {
                    ret.emplace(err(std::make_error_code(
                        std::errc::address_not_available)));
                } else {
                    ret.emplace(ok(results->endpoint().address()));
                }
                this->finish();
            });
    }

    result<ip_address> await_resume() { return *std::move(ret); }
};

future<result<ip_address>> resolve_one(std::string_view host) {
    resolve_one_any_future fut;
    fut.start(host);
    co_return co_await fut;
}

struct resolve_one_v4_future : public detail::io_future {
    asio::ip::tcp::resolver resolver;
    option<result<ipv4_address>> ret;

    resolve_one_v4_future() : resolver(detail::context()) {}

    void start(std::string_view host) {
        resolver.async_resolve(
            asio::ip::tcp::v4(), host, "", asio::ip::tcp::resolver::passive,
            [this](const auto& ec, auto results) {
                if (ec) {
                    ret.emplace(err(ec));
                } else if (results.empty()) {
                    ret.emplace(err(std::make_error_code(
                        std::errc::address_not_available)));
                } else {
                    ret.emplace(ok(results->endpoint().address().to_v4()));
                }
                this->finish();
            });
    }

    result<ipv4_address> await_resume() { return *std::move(ret); }
};

future<result<ipv4_address>> resolve_one_v4(std::string_view host) {
    resolve_one_v4_future fut;
    fut.start(host);
    co_return co_await fut;
}

struct resolve_one_v6_future : public detail::io_future {
    asio::ip::tcp::resolver resolver;
    option<result<ipv6_address>> ret;

    resolve_one_v6_future() : resolver(detail::context()) {}

    void start(std::string_view host) {
        resolver.async_resolve(
            asio::ip::tcp::v6(), host, "", asio::ip::tcp::resolver::passive,
            [this](const auto& ec, auto results) {
                if (ec) {
                    ret.emplace(err(ec));
                } else if (results.empty()) {
                    ret.emplace(err(std::make_error_code(
                        std::errc::address_not_available)));
                } else {
                    ret.emplace(ok(results->endpoint().address().to_v6()));
                }
                this->finish();
            });
    }

    result<ipv6_address> await_resume() { return *std::move(ret); }
};

future<result<ipv6_address>> resolve_one_v6(std::string_view host) {
    resolve_one_v6_future fut;
    fut.start(host);
    co_return co_await fut;
}

struct resolve_any_future : public detail::io_future {
    asio::ip::tcp::resolver resolver;
    option<result<std::vector<ip_address>>> ret;

    resolve_any_future() : resolver(detail::context()) {}

    void start(std::string_view host) {
        resolver.async_resolve(
            host, "", asio::ip::tcp::resolver::passive,
            [this](const auto& ec, auto results) {
                if (ec) {
                    ret.emplace(err(ec));
                } else {
                    std::vector<ip_address> addrs;
                    addrs.reserve(results.size());
                    for (const auto& result : results) {
                        addrs.emplace_back(result.endpoint().address());
                    }
                    ret.emplace(ok(std::move(addrs)));
                }
                this->finish();
            });
    }

    result<std::vector<ip_address>> await_resume() { return *std::move(ret); }
};

future<result<std::vector<ip_address>>> resolve(std::string_view host) {
    resolve_any_future fut;
    fut.start(host);
    co_return co_await fut;
}

struct resolve_v4_future : public detail::io_future {
    asio::ip::tcp::resolver resolver;
    option<result<std::vector<ipv4_address>>> ret;

    resolve_v4_future() : resolver(detail::context()) {}

    void start(std::string_view host) {
        resolver.async_resolve(
            asio::ip::tcp::v4(), host, "", asio::ip::tcp::resolver::passive,
            [this](const auto& ec, auto results) {
                if (ec) {
                    ret.emplace(err(ec));
                } else {
                    std::vector<ipv4_address> addrs;
                    addrs.reserve(results.size());
                    for (const auto& result : results) {
                        addrs.emplace_back(result.endpoint().address().to_v4());
                    }
                    ret.emplace(ok(std::move(addrs)));
                }
                this->finish();
            });
    }

    result<std::vector<ipv4_address>> await_resume() { return *std::move(ret); }
};

future<result<std::vector<ipv4_address>>> resolve_v4(std::string_view host) {
    resolve_v4_future fut;
    fut.start(host);
    co_return co_await fut;
}

struct resolve_v6_future : public detail::io_future {
    asio::ip::tcp::resolver resolver;
    option<result<std::vector<ipv6_address>>> ret;

    resolve_v6_future() : resolver(detail::context()) {}

    void start(std::string_view host) {
        resolver.async_resolve(
            asio::ip::tcp::v6(), host, "", asio::ip::tcp::resolver::passive,
            [this](const auto& ec, auto results) {
                if (ec) {
                    ret.emplace(err(ec));
                } else {
                    std::vector<ipv6_address> addrs;
                    addrs.reserve(results.size());
                    for (const auto& result : results) {
                        addrs.emplace_back(result.endpoint().address().to_v6());
                    }
                    ret.emplace(ok(std::move(addrs)));
                }
                this->finish();
            });
    }

    result<std::vector<ipv6_address>> await_resume() { return *std::move(ret); }
};

future<result<std::vector<ipv6_address>>> resolve_v6(std::string_view host) {
    resolve_v6_future fut;
    fut.start(host);
    co_return co_await fut;
}

} // namespace crasy
