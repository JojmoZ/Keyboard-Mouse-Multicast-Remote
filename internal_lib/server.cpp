#include "internal_lib.hpp"
#include <thread>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <iostream>
#include <memory>
#include <atomic>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

// =========================================================
// Utility: get local IPv4 address (not loopback)
// =========================================================
std::string GetLocalIPv4Address() {
    char ac[80];
    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
        return "";
    }
    struct hostent* phe = gethostbyname(ac);
    if (phe == nullptr) {
        return "";
    }
    for (int i = 0; phe->h_addr_list[i] != nullptr; ++i) {
        struct in_addr addr;
        memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
        std::string ip = inet_ntoa(addr);
        if (ip != "127.0.0.1") {
            return ip;
        }
    }
    return "";
}

// =========================================================
// Globals
// =========================================================
std::unique_ptr<UdpMulticastServer> serverInstance;
std::atomic<bool> running{false};
std::thread mouseSendThread;
// =========================================================
// Start Track Server
// =========================================================
int startTrackServer() {
    MouseCapture* capture = MouseCapture::GetInstance();
    std::thread poller(PollMouseWindows, std::ref(*capture));
    poller.detach();

    startHook();
    std::thread t(MessagePump);
    t.detach();

    KeyboardCapture* kCapture = KeyboardCapture::GetInstance();
    std::thread trackKey(startKeyboardTrack);
    trackKey.detach();

    try {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return -1;
        }

        asio::io_context io_context;
        std::string multicast_address = "239.255.0.1"; // Multicast group address
        unsigned short multicast_port = 8080;
 // 1. Create the server
        serverInstance = std::make_unique<UdpMulticastServer>(io_context, multicast_address, multicast_port);
        
        // 2. Start its internal threads
        serverInstance->start(10, capture, kCapture);

        running = true;

        // 3. The main thread now waits until it's told to stop
        while(running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Shutdown is handled by stopClient(), so we just clean up here.
        WSACleanup();


    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        running = false;
    }

    return 0;
}

// =========================================================
// Stop Client
// =========================================================
void stopClient() {
    if (serverInstance) {
        // This is the only function that should stop the server.
        // The close() method now correctly handles threads and sockets.
        serverInstance->close();
        serverInstance.reset();
    }
    // Signal the main loop in startTrackServer to exit
    running = false;
}

// =========================================================
// C Interface (for DLL or external linkage)
// =========================================================
extern "C" {
    int startTrackServerC() {
        return startTrackServer();
    }

    void stopClientC() {
        stopClient();
    }
}
