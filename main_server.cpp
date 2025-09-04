#include "internal_lib/internal_lib.hpp"
#include <iostream>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <vector>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

int main() {
    startTrackServer();
    return 0;
}