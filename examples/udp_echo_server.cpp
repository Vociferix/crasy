#include <crasy/crasy.hpp>
#include <vector>

#include "helpers.hpp"

// The server will listen on port 40044
inline constexpr crasy::port_type SERVER_PORT = 40044;

// The server will accept payloads up to 1500 bytes in size
inline constexpr std::size_t BUFFER_SIZE = 1500;

// Sends the received data back to the remote endpoint
crasy::future<void> echo(crasy::udp_socket& socket,
                         crasy::endpoint remote_endpoint,
                         std::vector<std::byte> data) {
    // Print the received data for demonstrative purposes
    print_data(data);

    // Wait for the reponse to be sent
    check_result(co_await socket.send_to(data, remote_endpoint));
}

// The async entrypoint
crasy::future<int> async_main() {
    crasy::endpoint local_endpoint(crasy::ipv4_address::any(), SERVER_PORT);
    crasy::endpoint remote_endpoint;
    crasy::udp_socket listen_socket;
    std::vector<std::byte> recv_buffer;

    // Bind the UDP socket to the local port to listen on
    auto bind_success = co_await listen_socket.bind_local(local_endpoint);
    if (!bind_success) { // handle errors
        auto err = bind_success.err().code();
        std::cerr << err.message() << " (" << err.value() << ")\n";
        co_return err.value();
    }

    // Listen loop
    for (;;) {
        remote_endpoint = crasy::endpoint(); // Reset remote endpoint

        // Wait to receive data on the UDP socket
        if (!check_result(co_await listen_socket.wait_read())) {
            co_return 1;
        }

        // Get the size of the received data and allocate enough space for it
        auto size = check_result(listen_socket.available());
        if (!size) {
            co_return 1;
        }
        recv_buffer.resize(*size);

        // Read the data from the UDP socket
        if (!check_result(co_await listen_socket.recv_from(recv_buffer, remote_endpoint))) {
            co_return 1;
        }

        // Spawn a new async task to respond - While this task is working
        // on sending the reponse, we can go back to listening for new
        // packets.
        crasy::spawn([=, &listen_socket] /* capture data by value! */ () mutable -> crasy::future<void> {
            co_await echo(listen_socket, remote_endpoint, std::move(recv_buffer));
        }).detach();
    }
    co_return 0;
}

int main() {
    // Create an executor that uses all available cores
    crasy::executor exec;

    // Run async_main on the executor
    return exec.block_on(async_main);
}
