#pragma once

#ifdef __cplusplus
#include <string> // This is needed for std::string
#include <vector>
#include "./network/Multicast.hpp"
#include "./mouse/Mouse.hpp"

// These are your C++-only functions
int startTrackServer();
std::string GetLocalIPv4Address();
void startClient(std::string listenAddr, int port);
#endif

// This is the pure C-style interface that Go can understand
#ifdef __cplusplus
extern "C" {
#endif

void startClientC(const char* listenAddr, int port);
int startTrackServerC();
void stopClientC();

#ifdef __cplusplus
}
#endif