#include "internal_lib.hpp"
#include <asio.hpp>
#include <iostream>

void startClient(std::string listenAddr, int port) {
    asio::io_context io_context;

    try {
        UdpMultiCastClient client(io_context, listenAddr, port);

        client.socket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), port));

        client.socket.set_option(
            asio::ip::multicast::join_group(asio::ip::make_address(listenAddr))
        );

        client.listen_loop();

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

extern "C" void startClientC(const char* listenAddr, int port) {
    std::string addr(listenAddr);
    startClient(addr, port);
}
