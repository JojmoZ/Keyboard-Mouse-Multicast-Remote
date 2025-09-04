#include "Multicast.hpp"
void  UdpMultiCastClient::listen_loop() {
    std::array<uint8_t, 1024> recv_buffer;
    asio::ip::udp::endpoint sender_endpoint;

    MouseState mState;
    MouseState prevMState;

    KeyboardState kState;
    while (true) {
        try {
            size_t bytes_received = socket.receive_from(
                asio::buffer(recv_buffer), sender_endpoint);

            if (bytes_received != 16) { // check if buffer is valid
              std::cout<<"Catch data with unrecognize bytesize"<<std::endl;
            }

            auto data = recv_buffer.data();
            if (isMouseData(data, 16)){
                parseMouseData(mState, data, bytes_received);
                  std::cout << "Received packet from " 
                              << sender_endpoint.address().to_string() << ": "
                              << "dx=" << mState.dx 
                              << ", dy=" << mState.dy 
                              << ", left=" << mState.leftClick
                              << ", right=" << mState.rightClick
                              << ", mid=" << mState.midClick
                              << std::endl;
                  
                  WinApplyMouseState(mState, prevMState);
            }else if (isKeyboardData(data, 16)){
                parseKeyboardData(kState, data, 16);
                std::cout
                    << "isKeyDown=" << kState.press 
                    << ", code=" << kState.code
                    << std::endl;
                WinApplyKeyInput(kState.press, kState.code );
            }else{
                CommandAction act = parseCommandData(data, 16);
                if (act == CommandAction::STOP_ACTION){
                    std::cout<<"Received STOP_ACTION command, exiting listen loop"<<std::endl;
                    return; // end program by return
                }
            }

            
        } catch (std::exception& e) {
            std::cerr << "Receive error: " << e.what() << std::endl;
        }
    }
}
