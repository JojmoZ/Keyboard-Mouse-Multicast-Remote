#include "internal_lib.hpp"
#include <thread>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <iostream>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

std::string GetLocalIPv4Address() {
    char ac[80];
    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
        return "";
    }
    struct hostent* phe = gethostbyname(ac);
    if (phe == 0) {
        return "";
    }
    for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
        struct in_addr addr;
        memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
        std::string ip = inet_ntoa(addr);
        if (ip != "127.0.0.1") {
            return ip;
        }
    }
    return "";
}

//BAD DESIGN, FIX IT LATER ; DANGLING PTR
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
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return -1;
        }

        asio::io_context io_context;
        std::string multicast_address = "239.255.0.1"; // Multicast group address
        unsigned short multicast_port = 8080;

        UdpMulticastServer server(io_context, multicast_address, multicast_port);

        //BAD DESIGN, FIX IT LATER
        serverInstance = &server;

        std::thread mouseSend([&]() {
            server.send_loop(10, capture);
        });
        mouseSend.detach();
        server.send_loop(10, kCapture);

        WSACleanup();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}


void stopClient(){
    if (serverInstance) {
        uint8_t buf[16] = {0};
        formatCommandData(CommandAction::STOP_ACTION, buf, sizeof(buf));
        serverInstance->send(buf,16);
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