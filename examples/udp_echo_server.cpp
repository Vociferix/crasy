#include <crasy/crasy.hpp>
#include <vector>

#include "helpers.hpp"

inline constexpr crasy::port_type SERVER_PORT = 40044;
inline constexpr std::size_t BUFFER_SIZE = 1500;

crasy::future<void> echo(crasy::udp_socket& socket,
                         crasy::endpoint remote_endpoint,
                         std::vector<std::byte> data) {
    print_data(data);
    check_result(co_await socket.send_to(data, remote_endpoint));
}

crasy::future<int> async_main() {
    crasy::endpoint local_endpoint(crasy::ipv4_address::any(), SERVER_PORT);
    crasy::endpoint remote_endpoint;
    crasy::udp_socket listen_socket;
    std::vector<std::byte> recv_buffer;

    auto bind_success = co_await listen_socket.bind_local(local_endpoint);
    if (!bind_success) {
        auto err = bind_success.err().code();
        std::cerr << err.message() << " (" << err.value() << ")\n";
        co_return err.value();
    }

    for (;;) {
        remote_endpoint = crasy::endpoint();
        recv_buffer.resize(BUFFER_SIZE);
        auto num_bytes = check_result(co_await listen_socket.recv_from(recv_buffer, remote_endpoint));
        if (!num_bytes) { continue; }
        recv_buffer.resize(*num_bytes);
        crasy::spawn([=, &listen_socket] /* capture by value! */ () mutable -> crasy::future<void> {
            co_await echo(listen_socket, remote_endpoint, std::move(recv_buffer));
        }).detach();
    }
    co_return 0;
}

int main() {
    return crasy::executor().block_on(async_main);
}
