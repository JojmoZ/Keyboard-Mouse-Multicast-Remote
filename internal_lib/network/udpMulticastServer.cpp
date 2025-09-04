#include "Multicast.hpp"


void UdpMulticastServer::send_loop(int interval_seconds, MouseCapture * mouseCapture) {
    MouseState state;
    uint8_t buf[16]={};
    while (true) {
        if (mouseCapture->poll(state)){
            formatMouseData(state, buf, 16);
            socket.send_to(asio::buffer(buf, 16), multicast_endpoint);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_seconds));
    }
}

void UdpMulticastServer::send_loop(int interval_seconds, KeyboardCapture * KeyboardCapture){
    KeyboardState state;
    uint8_t buf[16]={};
    while (true) {
        if (KeyboardCapture->poll(state)){
            formatKeyboardData(state, buf, 16);
            std::cout
                    << "isKeyDown=" << state.press 
                    << ", code=" << state.code
                    << std::endl;
            socket.send_to(asio::buffer(buf, 16), multicast_endpoint);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_seconds));
    }
}

void UdpMulticastServer::send(uint8_t *buf, int len) {
    socket.send_to(asio::buffer(buf, 16), multicast_endpoint);
}