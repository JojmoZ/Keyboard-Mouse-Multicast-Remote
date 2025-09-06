#include "Multicast.hpp"

void UdpMulticastServer::send_loop(int interval_ms, MouseCapture* mouseCapture) {
    MouseState state;
    uint8_t buf[16] = {};
    while (active) {
        if (mouseCapture->poll(state)) {
            formatMouseData(state, buf, 16);
            asio::error_code ec;
            if (socket.is_open())
                socket.send_to(asio::buffer(buf, 16), multicast_endpoint, 0, ec);
            if (ec) {
                std::cerr << "mouse send error: " << ec.message() << std::endl;
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    }
}

void UdpMulticastServer::send_loop(int interval_ms, KeyboardCapture* keyboardCapture) {
    KeyboardState state;
    uint8_t buf[16] = {};
    while (active) {
        if (keyboardCapture->poll(state)) {
            formatKeyboardData(state, buf, 16);
            std::cout << "isKeyDown=" << state.press
                      << ", code=" << state.code << std::endl;

            asio::error_code ec;
            if (socket.is_open())
                socket.send_to(asio::buffer(buf, 16), multicast_endpoint, 0, ec);
            if (ec) {
                std::cerr << "keyboard send error: " << ec.message() << std::endl;
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    }
}

void UdpMulticastServer::send(uint8_t* buf, int len) {
    if (!active || !socket.is_open()) return;
    asio::error_code ec;
    socket.send_to(asio::buffer(buf, len), multicast_endpoint, 0, ec);
    if (ec) {
        std::cerr << "send error: " << ec.message() << std::endl;
    }
}

void UdpMulticastServer::start(int interval_ms, MouseCapture* mouse, KeyboardCapture* keyboard) {
    active = true;
    if (mouse) {
        mouseThread = std::thread([this, interval_ms, mouse]() { 
            this->send_loop(interval_ms, mouse); 
        });
    }
    if (keyboard) {
        keyboardThread = std::thread([this, interval_ms, keyboard]() { 
            this->send_loop(interval_ms, keyboard); 
        });
    }
}

void UdpMulticastServer::close() {
    active = false;

    if (mouseThread.joinable()) {
        mouseThread.join();
    }
    if (keyboardThread.joinable()) {
        keyboardThread.join();
    }

    // 3. Close the socket *after* threads are done
    if (socket.is_open()) {
        asio::error_code ec;
        socket.close(ec);
        if (ec) {
            std::cerr << "socket close error: " << ec.message() << std::endl;
        }
    }
}