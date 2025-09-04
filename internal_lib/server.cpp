#include "internal_lib.hpp"
#include <thread>
#include <iostream>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

std::string GetLocalIPv4Address() {
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        return "";
    }

    std::string result = "";

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            int s = getnameinfo(
                ifa->ifa_addr,
                sizeof(struct sockaddr_in),
                host, NI_MAXHOST,
                nullptr, 0,
                NI_NUMERICHOST
            );
            if (s == 0) {
                std::string ip = host;
                if (ip != "127.0.0.1") {
                    result = ip;
                    break;
                }
            }
        }
    }

    freeifaddrs(ifaddr);
    return result;
}

// BAD DESIGN, FIX IT LATER ; DANGLING PTR
UdpMulticastServer* serverInstance = nullptr;

int startTrackServer() {
    MouseCapture *capture = MouseCapture::GetInstance();
    std::thread poller(PollMouseWindows, std::ref(*capture));
    poller.detach(); 

    startHook();
    std::thread t(MessagePump);

    KeyboardCapture *kCapture = KeyboardCapture::GetInstance();
    std::thread trackKey(startKeyboardTrack);
    trackKey.detach();

    try {

        asio::io_context io_context;
        std::string multicast_address = "239.255.0.1"; // Multicast group address
        unsigned short multicast_port = 8080;

        UdpMulticastServer server(io_context, multicast_address, multicast_port);

        serverInstance = &server;

        std::thread mouseSend([&]() {
            server.send_loop(10, capture);
        });
        mouseSend.detach();

        server.send_loop(10, kCapture);

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

void stopClient() {
    if (serverInstance) {
        uint8_t buf[16] = {0};
        formatCommandData(CommandAction::STOP_ACTION, buf, sizeof(buf));
        serverInstance->send(buf, 16);
    }
}

extern "C" {
    int startTrackServerC() {
        return startTrackServer();
    }

    void stopClientC() {
        stopClient();
    }
}
