#include "internal_lib/internal_lib.hpp"
#include <iostream>
#include <string>
int main(){
    std::string serverIP; 
    int port;
    std::cout << "Enter server IP: ";
    std::cin >> serverIP;
    std::cout << "Enter port: ";
    std::cin >> port;

    startClient(serverIP, port);
}